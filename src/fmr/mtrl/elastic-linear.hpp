#ifndef FMR_HAS_ELASTIC_GENERAL_HPP
#define FMR_HAS_ELASTIC_GENERAL_HPP
#include "../fmr.hpp"
#include <immintrin.h>

//FIXME Need maecros: FMR_HAS_AVX and FMR_HAS_AVX2

namespace fmr { namespace mtrl {
  //
  template <typename F> static inline//NOTE H volatile for performance testing
  void elas_dmat_base
    (F* stress, const F* D, volatile F* H, F* strain_voigt, F* stress_voigt)
    __attribute__((optimize ("O3")));// This is for performance testing.
  //
  template <typename F> static inline//NOTE H volatile for performance testing
  void elas_iso_lame
    (F* stress, const F lambda, const F mu, volatile F* H, F* HT)
    __attribute__((optimize ("O3")));
  //
  template <typename F> static inline//NOTE H volatile for performance testing
  void elas_iso_scalar
    (F* stress, const F c1, const F c2, const F c3, volatile F* H,
     F* stress_voigt)
    __attribute__((optimize ("O3")));
  //
  template <typename F> static inline//NOTE H volatile for performance testing
  void elas_iso_avx
    (F* stress, const F lambda, const F mu, volatile __m256d* vA)
    __attribute__((optimize ("O3")));
  //
  template <typename F> static inline//NOTE H volatile for performance testing
  void elas_iso_avx2
    (volatile __m256d* vA, const F lambda, const F mu)
    __attribute__((optimize ("O3")));
  //
} }//end namespace fmr::mtrl

#include "elastic-linear.ipp"

//end FMR_HAS_ELASTIC_GENERAL_HPP
#endif

