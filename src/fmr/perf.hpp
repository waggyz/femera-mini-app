#ifndef FMR_HAS_PERF_HPP
#define FMR_HAS_PERF_HPP

#include "fmr.hpp"

#include <chrono>   // high_resolution_clock, duration, nanoseconds; used in perf.ipp

namespace fmr { namespace perf {
// This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
// https://google.github.io/styleguide/cppguide.html#Aliases

#ifndef FMR_PERF_TOL
#define FMR_PERF_TOL 5
#endif
static const fmr::Perf_float perf_tol_rel
  = fmr::Perf_float(FMR_PERF_TOL) / fmr::Perf_float(100.0);

using Float     = fmr::Perf_float;
using Int       = fmr::Perf_int  ;
//
using Timepoint = Int            ;// ...One underlying type...
using Elapsed   = Int            ;// ...of these user types...
using Count     = Int            ;// ...used for simple struct packing.

static inline Timepoint get_now_ns () noexcept;

} }// end fmr::perf:: namespace

#include "perf.ipp"

//end FMR_HAS_PERF_HPP
#endif
