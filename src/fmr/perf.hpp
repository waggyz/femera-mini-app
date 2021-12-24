#ifndef FMR_HAS_PERF_HPP
#define FMR_HAS_PERF_HPP

#include "fmr.hpp"

namespace fmr { namespace perf {
// This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
// https://google.github.io/styleguide/cppguide.html#Aliases

using Float     = femera::Perf_float;
using Timepoint = femera::Perf_int  ;// ...One underlying type...
using Elapsed   = femera::Perf_int  ;// ...of these user types...
using Count     = femera::Perf_int  ;// ...used for easy struct packing.

static inline Timepoint get_now_ns () noexcept;

} }// end fmr::perf:: namespace

#include "perf.ipp"

#undef FMR_DEBUG
//end FMR_HAS_PERF_HPP
#endif
