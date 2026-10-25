#ifndef FMR_HAS_SMAT_HPP
#define FMR_HAS_SMAT_HPP
// Small matrix utilities

#include "math.hpp"

#include <immintrin.h>

namespace fmr { namespace math {
  /**
   * Matrix inverse functions return:
   *  1 for singular,
   * -1 for negative determinant
   *  0 for nonnegative det.
   * 
   *_ref variants are the gold standard for correctness,
   * and baseline for performance comparisons.
   *
   *_a, *_b,... are scalar-coded variants that may be more performant.
   *_avx, _avx2, _256, are variants optimized for specific SIMD instruction sets.
   *
   * Actual performance is tested in smat.perf.gtst so the fastest variant
   * can be identified empirically.
   */

template <typename F> static inline
int m1x1_inv_ref (const F* mat, F* inv);

template <typename F> static inline// matrix inverse & determinant
int m1x1_inv_det_ref (const F* mat, F* inv_det);

template <typename F> static inline// matrix inverse transposed & determinant
int m1x1_invt_det_ref (const F* mat, F* invt_det);

template <typename F> static inline
int m2x2_inv_ref (const F* mat, F* inv);
template <typename F> static inline
int m2x2_inv_a (const F* mat, F* inv);
template <typename F> static inline
int m2x2_inv_b (const F* mat, F* inv);

template <typename F> static inline// matrix inverse & determinant
int m2x2_inv_det_ref (const F* mat, F* inv_det);

template <typename F> static inline// matrix inverse transposed & determinant
int m2x2_invt_det_ref (const F* mat, F* invt_det);


template <typename F> static inline
F m3x3_det_ref (const F* mat);

template <typename F> static inline
int m3x3_inv_ref (const F* mat, F* inv);
template <typename F> static inline
int m3x3_inv_a (const F* mat, F* inv);

template <typename F> static inline// matrix inverse & determinant
int m3x3_inv_det_ref (const F* mat, F* inv_det);

template <typename F> static inline// matrix inverse transposed & determinant
int m3x3_invt_det_ref (const F* mat, F* invt_det);
template <typename F> static inline
int m3x3_invt_det_a (const F* mat, F* invt_det);

template <typename F> static inline
int m4x4_inv_ref (const F* mat, F* inv);

template <typename F> static inline// matrix inverse & determinant
int m4x4_inv_det_ref (const F* mat, F* inv_det);

template <typename F> static inline// matrix inverse transposed & determinant
int m4x4_invt_det_ref (const F* mat, F* invt_det);

#include "smat.ipp"

} } // namespace fmr::math

//end FMR_HAS_SMAT_HPP
#endif
