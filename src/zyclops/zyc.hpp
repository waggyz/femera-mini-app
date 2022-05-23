#ifndef ZYC_HAS_ZYC_HPP
#define ZYC_HAS_ZYC_HPP

#include <sys/types.h>
#include <cstdint>

#ifndef ZYC_STEP_SINGLE
#define ZYC_STEP_SINGLE 1e-8f
#endif
#ifndef ZYC_STEP_DOUBLE
#define ZYC_STEP_DOUBLE 1e-12
#endif
#ifndef ZYC_ALIGN
#define ZYC_ALIGN alignof(__m512d)
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
  // max uint needed to index into   CR matrix: 2^(2*(zorder_max+1)) - 1
  // max uint needed for matrix-free CR access: 2^(  (zorder_max+1)) - 1
  using Zindex_int = uint_fast32_t;
  using Zorder_int = Zindex_int;// same as Zindex_int; name is for semantics
  enum class Algebra : int8_t { Unknown =0,
    Real, Complex, Dual, Split, QCDa, Oti, User,// QCDa: Cayley-Dickson algebras
    Int,// signed integer
    Nat // unsigned natural number
  };
  enum class Stored : int8_t { Unknown =0,// for arrays of Zomplex numbers
    Mixed, // AoS (array of structs, interleaved real & imaginary parts)
    Block, // SoA (struct of arrays, arrays of real & each imaginary part)
    Native // Native is for real & built-in complex type
  };
  static const Zorder_int zorder_max = 15;
  //
  static inline
  Zindex_int upow (Zindex_int base, Zindex_int exp)
  noexcept;
  static inline constexpr
  uint hamw (uint64_t i)
  noexcept;
  static inline constexpr
  uint hamw (uint32_t i)
  noexcept;
  // multidual indexing
  static inline constexpr                      // returns true for nonzero
  bool is_mdcr_nz (Zindex_int r, Zindex_int c) // CR matrix form elements
  noexcept;
  static inline constexpr
  bool is_mdcr_nz (Zindex_int r, Zindex_int c, // also returns false when r,c
    Zorder_int zorder_stored)                  // is out of range, i.e.,
  noexcept;                                    // higher order than stored
  // multidual number operations
  template <typename T> static inline constexpr// returns a_rc,
  T mdcr_elem (const T& a,                     // fast with -flto
    Zindex_int row, Zindex_int col)
  noexcept;
  template <typename T> static inline constexpr// returns a_rc * b_rc,
  T mdcr_mult_elem (const T& a, const T& b,    // fast with -flto
    Zindex_int row, Zindex_int col)
  noexcept;
  template <typename T> static inline constexpr// returns a_rc with safe access
  T mdcr_elem (const T& a,                     // to higher order than stored
    Zindex_int row, Zindex_int col, Zorder_int zorder_stored)
  noexcept;
  // multidual array operations
  template <typename T> static inline constexpr// returns a_rc * b_rc
  T mdcr_mult_elem (const T& a, const T& b,    // for heterogenous order ops
    Zindex_int row, Zindex_int col, Zorder_int min_zorder_of_operands)
  noexcept;
  template <typename T> static inline          // c += a * b, returns c ptr,
  T* md_madd_aos (T& c, const T& a, const T& b,// interleaved real & imaginary
    Zorder_int order, std::size_t n=1)         // parts (Stored::Mixed)
  noexcept;
  template <typename T> static inline          // c += a * b, returns c ptr,
  T* md_madd_soa (T& c, const T& a, const T& b,// contiguous real & imaginary
    Zorder_int order, std::size_t n=1)         // parts (Stored::Block)
  noexcept;
#if 0
  template <typename T>
  static inline constexpr
  T md_derivative (const T& v, const Zorder_int o)
  noexcept { return (&v)[o]; }
  template <typename T, typename... Args>
  static inline constexpr
  T md_derivative (const T& v, const Args... orders)
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
