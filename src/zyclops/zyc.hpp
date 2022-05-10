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
#define ZYC_ALIGN_PTR __attribute__((aligned(ZYC_ALIGN))) auto* ZYC_RESTRICT

namespace zyc {

  using Zsize_t    = uint_fast32_t;// vector or array dimension
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
  static inline constexpr
  uint hamw (uint64_t i);
  static inline constexpr
  uint hamw (uint32_t i);
  static inline
  uint upow (uint base, uint exp);
  static inline constexpr
  uint dual_nz (uint row, uint col);
  static inline constexpr
  bool is_dual_nz (uint row, uint col);
  static inline constexpr
  bool is_dual_nz (uint row, uint col, uint order);
  static inline constexpr
  int dual_ix (int row, int col);//NOTE returns negative index for zero elements
  static inline constexpr
  uint dual_ux (uint row, uint col);//NOTE returns index zero for zero elements
  static inline constexpr
  int dual_ix (int row, int col, int order);
  static inline constexpr
  uint dual_tnz (uint row, uint col);// transpose
  static inline constexpr
  int dual_tix (int row, int col);// transpose
  static inline constexpr
  int dual_tix (int row, int col, int order);// transpose
  template <typename T> inline constexpr
  T dual_elem_cr (const T*, uint row=0, uint col=0);// slow

}// end zyc namespace

namespace zyc { namespace test {
  int gtst_main (int* argc, char** argv);
} }// end zyc::test:: namespace

#include "zyc.ipp"

//end ZYC_HAS_ZYC_HPP
#endif
