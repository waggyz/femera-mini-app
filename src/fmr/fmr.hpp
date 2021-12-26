#ifndef FMR_HAS_FMR_HPP
#define FMR_HAS_FMR_HPP

#include "../femera/femera.hpp"

#include <cstdint>

namespace fmr {
//NOTE prefer e.g. int_fast8_t, uint_least8_t, ...,  uintmax_t, uintptr_t
  
// This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
// https://google.github.io/styleguide/cppguide.html#Aliases

using Dim_int    = uint_fast8_t  ;// spatial dim., hier. depth, poly. order,...

using Perf_float = float         ;
using Perf_int   = uint_fast64_t ;// unit counters, internal time (ns) counters

using Exit_int   = int           ;// system return code

}//end fmr:: namespace

#undef FMR_DEBUG
//end FMR_HAS_FMR_HPP
#endif
