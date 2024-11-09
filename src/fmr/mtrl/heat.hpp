#ifndef FMR_HAS_THERMAL_HPP
#define FMR_HAS_THERMAL_HPP
#include "../fmr.hpp"

#include <type_traits>
#include <immintrin.h>

#ifdef FMR_HAS_MKL
#include <MKL.h>
#endif

namespace fmr { namespace mtrl { namespace thermal {
  //
} } }//end namespace fmr::mtrl::thermal

#include "heat.ipp"

//end FMR_HAS_THERMAL_HPP
#endif

