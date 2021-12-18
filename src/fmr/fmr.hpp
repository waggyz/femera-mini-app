#ifndef FMR_HAS_FMR_HPP
#define FMR_HAS_FMR_HPP

#include <cstdint>

namespace fmr {
// This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
// https://google.github.io/styleguide/cppguide.html#Aliases

using Dim_int = int_fast8_t;
//TODO e.g. int_fast8_t, uint_least8_t, ...,  uintmax_t, uintptr_t

}//end fmr:: namespace

#undef FMR_DEBUG
//end FMR_HAS_FMR_HPP
#endif