#ifndef ZYC_HAS_ZYCLOPS_HPP
#define ZYC_HAS_ZYCLOPS_HPP

#include <cstdint>

namespace zyc {

  using Zorder_int = uint8_t;
#ifdef FMR_HAS_ZYCLOPS
  enum class Algebra     : int8_t {Unknown =0,
    Real=zyc::Real, Complex=zyc::Complex, Dual=zyc::Dual, Split=zyc::Split,
    Quat=zyc::Quat,     OTI=zyc::OTI    , User=zyc::User,
    Int, Nat
  };
#else
  enum class Algebra     : int8_t {Unknown =0,
    Real, Complex, Dual, Split, Quat, OTI, User,
    Int,// signed
    Nat // unsigned
  };
#endif
}// end zyc namespace

//end ZYC_HAS_ZYCLOPS_HPP
#endif