#ifndef FMR_HAS_DETAIL_HPP
#define FMR_HAS_DETAIL_HPP

#include <cstdint>
#include <string>

namespace fmr { namespace detail {
// This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
// https://google.github.io/styleguide/cppguide.html#Aliases

using Perf_float = float;
using Perf_int   = uint_fast64_t;

} }//end fmr::detail:: namespace

namespace fmr { namespace detail { namespace form {

std::string si_unit_string (const double val, std::string unit,
  const int min_digits, const std::string signchar);

} } }// end fmr::form::detail:: namespace

#undef FMR_DEBUG
//end FMR_HAS_DETAIL_HPP
#endif
