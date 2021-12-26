#ifndef FMR_HAS_FMR_HPP
#define FMR_HAS_FMR_HPP

#include "../femera/femera.hpp"

#include <cstdint>

namespace fmr {
// This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
// https://google.github.io/styleguide/cppguide.html#Aliases

//NOTE prefer e.g. int_fast8_t, uint_least8_t, ...,  uintmax_t, uintptr_t
  
using Dim_int    = int_fast8_t;

using Exit_int   = int;

using Perf_float = float;
using Perf_int   = uint_fast64_t;

}//end fmr:: namespace

#undef FMR_DEBUG
//end FMR_HAS_FMR_HPP
#endif
