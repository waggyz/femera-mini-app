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

namespace zyc {
  using Zorder_int = uint_fast8_t;
#if 0
  using Zix_int    =  int_fast16_t;// CR matrix index, order <=  7
  using Zix_int    =  int_fast32_t;// CR matrix index, order <= 15
#endif
  using Zix_int    =  int_fast32_t;// CR matrix index, order <= 15
  using Zsize_t    =  int_fast64_t;// CR matrix size, >= 2^(2*order)
  enum class Algebra : int8_t {Unknown =0,
    Real, Complex, Dual, Split, Quat, OTI, User,
    Int,// signed
    Nat // unsigned
  };
  enum class Layout : int8_t { Unknown =0,// for arrays of Zomplex numbers
    Vector,// AoS (array of structs, interleaved real & imaginary parts)
    Block ,// SoA (struct of arrays, arrays of real & each imaginary part)
    Native // Native is for real & built-in complex type
  };
  static const Zorder_int zorder_max = 15;
  static inline
  uint upow (uint base, uint exp)
  noexcept;
  static inline constexpr
  uint hamw (uint64_t i)
  noexcept;
  static inline constexpr
  uint hamw (uint32_t i)
  noexcept;
  static inline constexpr
  bool is_dual_nz (Zix_int row, Zix_int col)
  noexcept;
  static inline constexpr
  bool is_dual_nz (Zix_int row, Zix_int col, Zix_int stored_zorder)
  noexcept;
#if 0
  template <typename T> static inline constexpr
  T cr_dual_elem (const T*, Zix_int row, Zix_int col)//TODO REMOVE
  noexcept;
  template <typename T> static inline constexpr//TODO REMOVE
  T cr_dual_elem (const T*, Zix_int row, Zix_int col, Zix_int stored_zorder)
  noexcept;// for access of higher order than stored
#endif
  template <typename T> static inline constexpr
  T cr_dual_elem (const T&, Zix_int row, Zix_int col)// fast with -flto
  noexcept;
  template <typename T> static inline constexpr
  T cr_dual_elem (const T&, Zix_int row, Zix_int col, Zix_int stored_zorder)
  noexcept;// for access of higher order than stored
#if 0
  template <typename T> static inline constexpr
  void dual_mult_aos //TODO Tune performance (permute loops?,
                 //     transposed mult? store rearranged?)
    (T* ZYC_RESTRICT a, const T* ZYC_RESTRICT b, const T* ZYC_RESTRICT c,
    zyc::Zorder_int order)
  noexcept;
#endif
#if 0
  template<typename T>
  static inline constexpr
  T dual_derivative (const T& v, const Zorder_int o)
  noexcept { return (&v)[o]; }
  template<typename T, typename... Args>
  static inline constexpr
  T dual_derivative (const T& v, const Args... orders)
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
