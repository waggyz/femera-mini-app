#ifndef ZYC_HAS_ZYC_HPP
#define ZYC_HAS_ZYC_HPP

#include <sys/types.h>
#include <cstdint>

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
#if 0
  using Zsize_t    = uint_fast32_t;// vector or array dimension order <=32
#else
  using Zsize_t    = uint_fast16_t;// vector or array dimension order <=16
#endif
  using Zorder_int = uint8_t;
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
  static inline
  uint upow (uint base, uint exp)
  noexcept;
  static inline constexpr
  uint hamw (uint64_t i)
  noexcept;
  static inline constexpr
  uint hamw (uint32_t i)
  noexcept;
  template <typename T> inline constexpr
  T cr_dual_elem (const T*, Zsize_t row, Zsize_t col)// fast with -flto
  noexcept;
  template <typename T> inline constexpr
  T cr_dual_elem (const T*, Zsize_t row, Zsize_t col, Zsize_t stored_order)
  noexcept;// safe for access of higher order than stored

}// end zyc namespace

namespace zyc { namespace test {
  int gtst_main (int* argc, char** argv);
} }// end zyc::test:: namespace

#include "zyc.ipp"

//end ZYC_HAS_ZYC_HPP
#endif
