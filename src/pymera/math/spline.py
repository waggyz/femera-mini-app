#!/usr/bin/env python3

import numpy as np
from scipy.interpolate import CubicSpline

"""
Spline math utilities to prototype high-performance general and specialized
spline algorithms.
"""

def eval_cubic_spline_ref(x, spline_data):
    """
    Evaluate a cubic spline at a given point x using the provided spline data.

    Parameters
    ----------
    x : float
        The point at which to evaluate the spline.
    spline_data : numpy.ndarray
        A float array containing spline knots and coefficients as:
        [[knot_0, a_0, b_0, c_0, d_0],
         [knot_1, a_1, b_1, c_1, d_1],
         ...,
         [knot_n, a_n, b_n, c_n, d_n]]

    Returns
    -------
    float
        The value of the cubic spline at the point x.

    Notes
    -----
    Exact knots select the interval to their left; extrapolation uses the
    first or last row. NumPy's polynomial evaluator uses Horner arithmetic,
    avoiding explicit squared and cubed powers that can overflow needlessly.
    """
    # Extract the knots and coefficients from the spline data
    knots = spline_data[:, 0]
    coefficients = spline_data[:, 1:]

    # Find the interval containing x
    i = 0
    while i < len(knots) - 1 and x > knots[i + 1]:
        i += 1

    # Evaluate the cubic spline using the coefficients for the interval
    dx = x - knots[i]
    return float(np.polynomial.polynomial.polyval(dx, coefficients[i]))


def eval_cubic_spline_knot_4_avx(x, spline_data, eval_as='ref'):
    """
    Prototype to simulate AVX-optimized 4-knot cubic spline double-precision evaluation.

    Parameters
    ----------
    x : float (double)
        The point at which to evaluate the spline.
    spline_data : numpy.ndarray (double)
        A float array containing spline knots and coefficients as:
        [[knot_0, a_0, b_0, c_0, d_0],
         [knot_1, a_1, b_1, c_1, d_1],
         [knot_2, a_2, b_2, c_2, d_2],
         [knot_3, a_3, b_3, c_3, d_3]]
    eval_as : str, optional
        The evaluation mode, either 'ref' for reference or
        labels to choose among various test spline data layouts and
        evaluation strategies for AVX optimization.
        Default is 'ref'.

    Notes
    -----
    Layout A evaluates one interval using coefficient and power vectors;
    layout B evaluates all four intervals using lane-wise Horner arithmetic.
    Layout C evaluates a selected pair of intervals using Estrin arithmetic,
    combining the low and high 128-bit halves without a horizontal reduction.
    Knots must be in ascending order. Exact knots use the interval to their
    left, and out-of-range inputs use the first or last coefficient row,
    matching the reference.

    The printed counters model a possible AVX implementation, not NumPy
    execution time. Layout packing is setup work and is excluded. Memory
    operations include loads, broadcasts, lane shuffles and scalar extraction;
    integer operations model movemask, shift and popcount for interval lookup.
    Layout C also counts pair selection, coefficient offset and lane selection.
    Floating-point operations count active lanes, without FMA contraction.
    A native benchmark should reuse prepacked data across scalar evaluations.

    Returns
    -------
    float
        The value of the cubic spline at the point x.
    """
    mops=0 # Register/memory operations counter for AVX simulation
    iops=0 # Integer operations counter for integer operations in AVX simulation
    flops = 0 # Floating-point operations counter for AVX simulation

    result = None
    if eval_as == 'ref':
        # Reference implementation
        result = eval_cubic_spline_ref(x, spline_data)
    elif eval_as == 'layout_avx_a':
        """
        Rearrange spline_data as:
        [[knot_0, knot_1, knot_2, knot_3],
        [a_0, b_0, c_0, d_0],
        [a_1, b_1, c_1, d_1],
        [a_2, b_2, c_2, d_2],
        [a_3, b_3, c_3, d_3]]
        """
        data_avx = np.zeros((5, 4), dtype=spline_data.dtype)
        data_avx[0, :] = spline_data[:, 0]
        data_avx[1, :] = spline_data[0, 1:]
        data_avx[2, :] = spline_data[1, 1:]
        data_avx[3, :] = spline_data[2, 1:]
        data_avx[4, :] = spline_data[3, 1:]
        # Load x into a 4-element array for AVX processing
        x_avx = np.full(4, x, dtype=spline_data.dtype)
        mops += 1
        #
        knots = data_avx[0, :]
        # free pointer alias.
        mops += 1
        #
        x_gt = x_avx > knots
        flops += 4
        #
        interval = int(np.count_nonzero(x_gt[1:]))
        iops += 3
        dx = x - knots[interval]
        mops += 1
        flops += 1
        dx_squared = dx * dx
        dx_cubed = dx_squared * dx
        flops += 2
        powers = np.array([1.0, dx, dx_squared, dx_cubed],
                  dtype=spline_data.dtype)
        mops += 1
        products = data_avx[interval + 1, :] * powers
        mops += 1
        flops += 4
        result = float((products[0] + products[1]) +
                   (products[2] + products[3]))
        mops += 3
        flops += 3

    elif eval_as == 'layout_avx_b':
        """
        Rearrange spline_data as:
        [[knot_0, knot_1, knot_2, knot_3],
        [a_0, a_1, a_2, a_3],
        [b_0, b_1, b_2, b_3],
        [c_0, c_1, c_2, c_3],
        [d_0, d_1, d_2, d_3]]
        """
        data_avx = np.zeros((5, 4), dtype=spline_data.dtype)
        data_avx[0, :] = spline_data[:, 0]
        data_avx[1, :] = spline_data[:, 1]
        data_avx[2, :] = spline_data[:, 2]
        data_avx[3, :] = spline_data[:, 3]
        data_avx[4, :] = spline_data[:, 4]
        x_avx = np.full(4, x, dtype=spline_data.dtype)
        knots = data_avx[0, :]
        mops += 2
        x_gt = x_avx > knots
        flops += 4
        interval = int(np.count_nonzero(x_gt[1:]))
        iops += 3
        dx = x_avx - knots
        flops += 4
        values = ((data_avx[4, :] * dx + data_avx[3, :]) * dx +
              data_avx[2, :]) * dx + data_avx[1, :]
        mops += 4
        flops += 24
        result = float(values[interval])
        mops += 1
    elif eval_as == 'layout_avx_c':
        """
        Rearrange spline_data as:
        [[knot_0, knot_1, knot_2, knot_3],
         [a_0, a_1, c_0, c_1],
         [b_0, b_1, d_0, d_1],
         [a_2, a_3, c_2, c_3],
         [b_2, b_3, d_2, d_3]]
        """
        data_avx = np.zeros((5, 4), dtype=spline_data.dtype)
        data_avx[0, :] = spline_data[:, 0]
        for pair_start in (0, 2):
            pair_data = spline_data[pair_start:pair_start + 2, :]
            data_avx[pair_start + 1, :] = pair_data[:, 1:4:2].T.reshape(4)
            data_avx[pair_start + 2, :] = pair_data[:, 2:5:2].T.reshape(4)
        x_avx = np.full(4, x, dtype=spline_data.dtype)
        knots = data_avx[0, :]
        mops += 2
        x_gt = x_avx > knots
        flops += 4
        interval = int(np.count_nonzero(x_gt[1:]))
        iops += 3
        pair_start = interval & ~1
        coefficient_row = pair_start + 1
        lane = interval & 1
        iops += 3
        pair_knots = np.tile(knots[pair_start:pair_start + 2], 2)
        mops += 1
        dx = x_avx - pair_knots
        flops += 4
        terms = data_avx[coefficient_row + 1, :] * dx + data_avx[coefficient_row, :]
        mops += 2
        flops += 8
        dx_squared = dx[:2] * dx[:2]
        flops += 2
        values = terms[2:] * dx_squared + terms[:2]
        mops += 1
        flops += 4
        result = float(values[lane])
        mops += 1
    else:
        raise NotImplementedError(
            f"AVX-optimized evaluation '{eval_as}' not implemented yet.")
    
    print(f"eval: {eval_as}, mops: {mops}, iops: {iops}, flops: {flops}")
    return result

def eval_origin_linear_to_cubic_spline_knot_XXX_avx(
        x, spline_data, eval_as_list=['layout_avx_a']):
    """
    Special-purpose linear-to-cubic spline evaluation useful for
    elastic-plastic material stress-strain curves. The first segment is linear
    through the origin and subsequent segments are cubic. The result is continuous,
    though it may have a discontinuous derivative at the transition point (sigma_y).
    Negative inputs are allowed and use the linear segment from the origin.
    Extrapolation beyond the last knot uses the cubic segment.
    """