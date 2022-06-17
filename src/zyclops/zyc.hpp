#ifndef ZYC_HAS_ZYC_HPP
#define ZYC_HAS_ZYC_HPP

#include <sys/types.h>
#include <cstdint>
#include <immintrin.h>       //__m256d, __m512d,...

#ifndef ZYC_STEP_SINGLE
#define ZYC_STEP_SINGLE 1e-8f
#endif
#ifndef ZYC_STEP_DOUBLE
#define ZYC_STEP_DOUBLE 1e-12
#endif
#ifndef ZYC_ALIGN
//TODO support x_HAS_AVX, x_HAS_AVX512
//#define ZYC_ALIGN alignof(__m512d)
#define ZYC_ALIGN alignof(__m256d)
#endif

#ifndef ZYC_RESTRICT
#define ZYC_RESTRICT __restrict
#endif
#define ZYC_ARRAY_PTR auto* ZYC_RESTRICT
#define ZYC_CONST_PTR const auto* ZYC_RESTRICT
#define ZYC_ALIGN_PTR auto* ZYC_RESTRICT __attribute__((aligned(ZYC_ALIGN)))

#ifdef _OPENMP
#define ZYC_HAS_OPENMP
#define ZYC_PRAGMA_OMP(x) _Pragma (#x)
#else
#define ZYC_PRAGMA_OMP(x) // pragma omp not supported
#endif

namespace zyclops {
  using Zindex_int = std::uint_fast32_t;// for indices into one CR matrix
  using Zorder_int = Zindex_int;// same as Zindex_int; name is for semantics
  // max uint needed to index into   CR matrix: 2^(2*(max_zorder+1)) - 1
  // max uint needed for matrix-free CR access: 2^(  (max_zorder+1)) - 1
  //
  enum class Algebra : std::int_fast8_t { Unknown =-1,
    Real, Complex, Dual, Split, Qcda, Oti, User,// Qcda: Cayley-Dickson algebras
    Integer,// signed integer
    Natural // unsigned natural number
  };
  enum class Layout : std::int_fast8_t { Unknown =-1,// for arrays of Zomplex
    Native,// Native is for real & built-in complex type
    Inset, // AoS (array of structs: interleaved real & imaginary parts)
    Block, // SoA (struct of arrays:  contiguous real & imaginary parts)
    User
  };
  static const Zorder_int max_zorder = (sizeof (Zindex_int) * 4) - 1;// 15;
  //
  static inline
  Zindex_int upow (Zindex_int base, Zindex_int exponent)
  noexcept;
#if 0
  static inline constexpr
  uint hamw (uint64_t)//TODO hamming weight
  noexcept;
  static inline constexpr
  uint hamw (uint32_t)
  noexcept;
#endif
  // multidual Cauchy-Riemann (CR) matrix indexing
  static inline constexpr                      // returns true for nonzero
  bool is_mdcr_nz (Zindex_int r, Zindex_int c) // CR matrix form elements
  noexcept;
  static inline constexpr
  bool is_mdcr_nz (Zindex_int r, Zindex_int c, // also returns false when r,c
    Zorder_int zorder_stored)                  // is out of range, i.e.,
  noexcept;                                    // higher order than stored
  // multidual number CR element operations
  template <typename T> static inline constexpr// returns a[row,col] (a_rc),
  T mdcr_elem (const T& a,                     // fast with -flto
    Zindex_int row, Zindex_int col)
  noexcept;
  template <typename T> static inline constexpr// returns a_rc with safe access
  T mdcr_elem (const T& a,                     // to higher order than stored
    Zindex_int row, Zindex_int col, Zorder_int zorder_stored)
  noexcept;
  template <typename T> static inline constexpr// returns a_r * b_rc,
  T mdcr_mult_elem (const T& a, const T& b,    // fast with -flto
    Zindex_int row, Zindex_int col)
  noexcept;
#if 0
  template <typename T> static inline constexpr// returns a_r * b_rc
  T mdcr_mult_elem (const T& a, const T& b,//TODO for heterogeneous order ops
    Zindex_int row, Zindex_int col, Zorder_int min_zorder_of_operands)
  noexcept;
#endif
  // hypercomplex scalar array operations
  template <typename T> static inline          // c = a + b, returns c ptr,
  T* hca_add (T& c, const T& a, const T& b, Zorder_int order, std::size_t n=1)
  noexcept;
  template <typename T> static inline          // c += a + b, returns c ptr,
  T* hca_adda (T& c, const T& a, const T& b, Zorder_int order, std::size_t n=1)
  noexcept;
  template <typename T> static inline          // c = a - b, returns c ptr,
  T* hca_sub (T& c, const T& a, const T& b, Zorder_int order, std::size_t n=1)
  noexcept;
  template <typename T> static inline          // c += a - b, returns c ptr,
  T* hca_suba (T& c, const T& a, const T& b, Zorder_int order, std::size_t n=1)
  noexcept;
  // multidual scalar array operations
  template <typename T> static inline          // c += a * b, returns c ptr,
  T* mdas_madd (T& c, const T& a, const T& b,  // interleaved real & imaginary
    Zorder_int order, std::size_t=1)           // parts (Layout::Inset)
  noexcept;
  template <typename T> static inline          // c += a * b, returns c ptr,
  T* mdsa_madd (T& c, const T& a, const T& b,  // contiguous real & imaginary
    Zorder_int order, std::size_t=1)           // parts (Layout::Block)
  noexcept;
  template <typename T> static inline          // c = a / b, returns c ptr,
  T* mdas_div (T& c, const T& a, const T& b,   // interleaved real & imaginary
    Zorder_int order, std::size_t=1)           // parts (Layout::Inset)
  noexcept;//NOTE result (c) must be zeroed before dividing
  template <typename T> static inline          // c = a / b, returns c ptr,
  T* mdsa_div  (T& c, const T& a, const T& b,  // contiguous real & imaginary
    Zorder_int order, std::size_t=1)           // parts (Layout::Block)
  noexcept;//NOTE result (c) must be zeroed before dividing
#if 0
  template <typename T>
  static inline constexpr
  T hc_derivative (const T& v, const Zorder_int o)
  noexcept { return (&v)[o]; }
  template <typename T, typename... Args>
  static inline constexpr
  T hc_derivative (const T& v, const Args... orders)
  noexcept { return get_derivative (v, orders...); }
#endif
#if 0
  static inline constexpr
  float get_deriv (const float& v, const float step_size=ZYC_STEP_SINGLE,
    Zorder_int=0)
  noexcept;
  template<typename... Orders>
  static inline constexpr
  float get_deriv (const float& v, const float step_size=ZYC_STEP_SINGLE,
    Orders... orders)
  noexcept { return get_deriv (v, step_size, orders...); }
#endif
#if 0
  static inline constexpr
  double get_deriv (const double&, const double step_size=ZYC_STEP_DOUBLE,
    Zorder_int=0);
  noexcept;
#endif

}// end zyc namespace

#include "zyc.ipp"

//end ZYC_HAS_ZYC_HPP
#endif
