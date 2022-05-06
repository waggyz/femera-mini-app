#ifndef ZYC_HAS_ZYCLOPS_HPP
#define ZYC_HAS_ZYCLOPS_HPP

#include <cstdint>

#define ZYC_ALIGN_INTS alignof(size_t)
#define ZYC_ALIGN_VALS alignof(__m256d)

#ifndef ZYC_RESTRICT
#define ZYC_RESTRICT __restrict
#endif
#define ZYC_ARRAY_PTR auto* ZYC_RESTRICT
#define ZYC_CONST_PTR const auto* ZYC_RESTRICT
#define ZYC_ALIGN_PTR __attribute__((aligned(ZYC_ALIGN_VALS))) auto* ZYC_RESTRICT

namespace zyc {

  using Zorder_int = uint8_t;
  enum class Algebra     : int8_t {Unknown =0,
    Real, Complex, Dual, Split, Quat, OTI, User,
    Int,// signed
    Nat // unsigned
  };
  enum class Layout      : int8_t { Unknown =0,// for arrays of Zomplex numbers
    Vector=1,// AoS (array of structs, interleaved real & imaginary parts)
    Block =2,// SoA (struct of arrays, arrays of real & each imaginary part)
    Native=3 // Native is for real & built-in complex type
  };
}// end zyc namespace

//end ZYC_HAS_ZYCLOPS_HPP
#endif