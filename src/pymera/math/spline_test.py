#!/usr/bin/env python3

import numpy as np
import contextlib
import io

from spline import (
    eval_cubic_spline_ref, eval_cubic_spline_knot_4_avx
    )
def test_cubic_spline_knot_4_avx(n_tests=256,
    eval_as_list=['ref', 'layout_avx_a', 'layout_avx_b', 'layout_avx_c']):
    """Check selected layouts against an independent float64 spline oracle.

    Fixed cases always run; n_tests is the number of additional reproducible
    randomized four-row splines. Inputs and coefficients are finite doubles.
    Exact knots, adjacent doubles, interval interiors and extrapolation are
    checked for every spline. Assertions identify the case, input and layout.
    Evaluator operation-counter output is suppressed. Success returns None.

    The oracle selects intervals independently and accumulates polynomial
    terms in long-double precision. Error tolerances account for float64
    rounding and cancellation using the sum of absolute term magnitudes.
    Reference-only regressions also check avoidable power overflow/underflow.
    """
    if not isinstance(n_tests, (int, np.integer)) or n_tests < 0:
        raise ValueError('n_tests must be a nonnegative integer')
    layouts = tuple(eval_as_list)
    if not layouts:
        raise ValueError('eval_as_list must contain at least one layout')
    rng = np.random.default_rng(20261015)
    tolerance = 64 * np.finfo(np.float64).eps

    with contextlib.redirect_stdout(io.StringIO()):
        extreme_data = np.zeros((4, 5), dtype=np.float64)
        extreme_data[:, 0] = [0.0, 1.0, 2.0, 3.0]
        extreme_data[:, 4] = 1e-300
        for point in (-1e110, 1e110):
            np.testing.assert_allclose(
                eval_cubic_spline_ref(point, extreme_data),
                np.copysign(1e30, point), rtol=1e-14, atol=0.0,
                err_msg='reference: avoidable cubic-power overflow')
        extreme_data[:, 4] = 1e300
        np.testing.assert_allclose(
            eval_cubic_spline_ref(-1e-110, extreme_data), -1e-30,
            rtol=1e-14, atol=0.0,
            err_msg='reference: avoidable cubic-power underflow')

        knots = np.array([-3.0, -0.5, 1.0, 4.0], dtype=np.float64)
        data = np.zeros((4, 5), dtype=np.float64)
        data[:, 0] = knots
        _check_spline(data, 'zero polynomial', tolerance, layouts)

        data[:, 1] = [1.0, -2.0, 3.0, -4.0]
        _check_spline(data, 'discontinuous interval constants', tolerance, layouts)

        for interval in range(4):
            for degree in range(4):
                data[:, 1:] = 0.0
                data[interval, degree + 1] = (-1.0) ** (interval + degree)
                _check_spline(data,
                    f'isolated interval={interval}, degree={degree}',
                    tolerance, layouts)

        data[:, 1] = 2.0 - 3.0 * knots + 4.0 * knots**2 - 0.5 * knots**3
        data[:, 2] = -3.0 + 8.0 * knots - 1.5 * knots**2
        data[:, 3] = 4.0 - 1.5 * knots
        data[:, 4] = -0.5
        _check_spline(data, 'global cubic in local coordinates', tolerance, layouts)
        _check_spline(np.asfortranarray(data), 'Fortran-contiguous storage', tolerance, layouts)
        storage = np.zeros((4, 10), dtype=np.float64)
        storage[:, ::2] = data
        _check_spline(storage[:, ::2], 'strided column storage', tolerance, layouts)
        _check_spline(data[::-1].copy()[::-1], 'negative row stride', tolerance, layouts)

        data[:, 0] = [0.0, np.nextafter(0.0, 1.0), 1.0,
                      np.nextafter(1.0, 2.0)]
        _check_spline(data, 'adjacent representable knots', tolerance, layouts)

        data[:, 0] = knots
        data[:, 1:] = [1.0, -3.0, 3.0, -1.0]
        _check_spline(data, 'cancellation near a triple root', tolerance, layouts,
                     np.concatenate((knots + 1.0,
                                     knots + (1.0 - 1e-5),
                                     knots + (1.0 + 1e-5))))

        for sample in range(n_tests):
            knot_scale = (1e-6, 1.0, 1e6)[sample % 3]
            coefficient_scale = (1e-6, 1.0, 1e6)[(sample // 3) % 3]
            knots = (np.cumsum(rng.uniform(0.125, 2.0, 4)) - 4.0) * knot_scale
            data = np.column_stack((
                knots, rng.normal(size=(4, 4)) * coefficient_scale))
            points = rng.uniform(knots[0] - knot_scale,
                                 knots[-1] + knot_scale, 16)
            _check_spline(data, f'random spline {sample}', tolerance, layouts, points)

def _check_spline(data, case, tolerance, layouts, extra_points=()):
    original = data.copy()
    knots = original[:, 0]
    span = knots[-1] - knots[0]
    points = np.concatenate((
        knots,
        np.nextafter(knots, -np.inf),
        np.nextafter(knots, np.inf),
        knots[:-1] + np.diff(knots) * 0.5,
        [knots[0] - span, knots[-1] + span, -0.0, 0.0],
        extra_points))
    for point in points:
        interval = int(np.searchsorted(knots[1:], point, side='left'))
        dx = np.longdouble(point) - np.longdouble(knots[interval])
        terms = original[interval, 1:].astype(np.longdouble) * np.array(
            [1.0, dx, dx * dx, dx * dx * dx], dtype=np.longdouble)
        expected = float(np.sum(terms))
        scale = max(1.0, float(np.sum(np.abs(terms))))
        reference = eval_cubic_spline_ref(point, data)
        np.testing.assert_allclose(
            reference, expected, rtol=tolerance, atol=tolerance * scale,
            equal_nan=False, err_msg=f'{case}, x={point!r}, reference')
        for layout in layouts:
            message = f'{case}, x={point!r}, layout={layout}'
            actual = eval_cubic_spline_knot_4_avx(point, data, layout)
            result_array = np.asarray(actual)
            assert result_array.shape == (), message + ': nonscalar result'
            assert result_array.dtype == np.dtype(np.float64), (
                message + ': result is not double precision')
            np.testing.assert_allclose(
                actual, expected, rtol=tolerance, atol=tolerance * scale,
                equal_nan=False, err_msg=message)
            np.testing.assert_array_equal(data, original, err_msg=message)
