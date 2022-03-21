#ifndef FEMERA_GEOM_HPP
#define FEMERA_GEOM_HPP

#include "Work.hpp"

#include <algorithm>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera { namespace geom {
#if 0
  //TODO much easier if stored as:
  //     float  jac[0-8], float  det, double detinv
  //  or double jac[0-8], double det, double detinv
  //     then load jac into padded array; det and detinv into local variables
  template <typename F>//TODO enable_if float, double
  struct Jac_det_pad {
    char buf [12*sizeof(F)];
#if 0
    F* get_jac () {return reinterpret_cast<F*> (&buf[0]);}
#else
    constexpr const F* get_jac () {
      return reinterpret_cast<const F*> (&buf[0]);
    }
#endif
    constexpr const F* get_det () {
      return reinterpret_cast<const F*> (&buf[11*sizeof(F)]);
    }
    constexpr void get_inv () {
      (sizeof(F) < sizeof(double))
      return reinterpret_cast<const F*> (&buf[11*sizeof(F)]);
    }
#if 0
    // The following will not work
    // F is float: concatenate bytes starting at 3&7 then cast to double
    // F is double: cast bytes starting at position 3 to double.
    static constexpr double inv_det () {return static_cast<double>(
      (sizeof(F) < sizeof(double))
      ? long(buf[3*sizeof(F)]) <<sizeof(F) + long(buf[7*sizeof(F)] <<sizeof(F))
      : long(buf[3*sizeof(F)]));}
#endif
  };
#endif
} }//end femera::geom:: namespace
#undef FMR_DEBUG

//#include "Geom.ipp"

//end FEMERA_GEOM_HPP
#endif
