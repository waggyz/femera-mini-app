#ifndef FMR_HAS_ELASTIC_LINEAR_HPP
#define FMR_HAS_ELASTIC_LINEAR_HPP
#include "../fmr.hpp"

#include <type_traits>
#include <immintrin.h>

#ifdef FMR_HAS_MKL
#include <MKL.h>
#endif

namespace fmr { namespace mtrl { namespace elastic {
  //
  template <typename F>
  using Isotropic_lame_parameters  = F[ 2];// lambda, mu (shear modulus, C44)
  template <typename F>                // C11=C22=C33, C44=C55=C66, C12=C13=C23
  using DMAT_cubic_constants       = F[ 3];// [C11 C44 C12], includes isotropic
#if 0
  template <typename F>              // C11=C22, C33, C44=C55, C66, C12, C13=C23
  using Hexagonal_dmat_constants   = F[ 6];// 5 independent, C66=(C11-C12)*0.5
  // Tetragonal, Rhombohedral: 7
#endif
  template <typename F>           // C11, C22, C33, C44, C55, C66, C12, C13, C23
  using DMAT_orthotropic_constants = F[ 9];//inc. orthorhombic crystal structure
  template <typename F>
  using DMAT_symm                  = F[21];// Triclinic crystal structure
  template <typename F>
  using DMAT_full                  = F[36];
  //
  template <typename F,///NOTE H volatile for performance testing
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr>
  static inline
  void linear_3d_dmat_base
    (F* stress, const F* D, volatile F* H, F* strain_voigt, F* stress_voigt)
    __attribute__((optimize ("O3")));// This is for performance testing.
  //
#ifdef FMR_HAS_MKL
  //TODO placeholder for Intel oneMKL symmetric packed (spmv) version
  template <typename F,///NOTE H volatile for performance testing
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr>
  static inline
  void linear_3d_dmat_spmv
    (F* stress, const F* D, volatile F* H, F* strain_voigt, F* stress_voigt)
    __attribute__((optimize ("O3")));
#endif
  //
  template <typename F,///NOTE H volatile for performance testing
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr>
  static inline
  void linear_3d_isotropic_lame
    (F* stress, const F lambda, const F mu, volatile F* H, F* HT)
    __attribute__((optimize ("O3")));
  //
#ifdef FMR_HAS_AVX
  template <typename F,///NOTE vH volatile for performance testing
  typename std::enable_if<std::is_same<F, double>::value>::type* = nullptr>
  static inline
  void linear_3d_isotropic_avx
    (F* stress, const F lambda, const F mu, volatile __m256d* vH)
    __attribute__((optimize ("O3")));
#endif
  //
#ifdef FMR_HAS_AVX2
  template <typename F,
  typename std::enable_if<std::is_same<F, double>::value>::type* = nullptr>
  static inline//NOTE vA volatile for performance testing
  // Strain goes into vA, stress comes out aofs vA
  void linear_3d_isotropic_avx2
    (volatile __m256d* vA, const F lambda, const F mu)
    __attribute__((optimize ("O3")));
#endif
  //
  template <typename F,///NOTE H volatile for performance testing
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr>
  static inline
  void linear_3d_cubic_scalar_a
    (F* stress, const F c1, const F c2, const F c3, volatile F* H)
    __attribute__((optimize ("O3")));
  //
  template <typename F,///NOTE H volatile for performance testing
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr>
  static inline
  void linear_3d_cubic_scalar_b
    (F* stress, const F c1, const F c2, const F c3, volatile F* H,
     F* stress_voigt)
    __attribute__((optimize ("O3")));
} } }//end namespace fmr::mtrl::elastic

#include "elastic-linear.ipp"

//end FMR_HAS_ELASTIC_LINEAR_HPP
#endif

