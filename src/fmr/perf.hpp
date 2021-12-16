#ifndef FMR_HAS_PERF_HPP
#define FMR_HAS_PERF_HPP

#include "detail.hpp"

#include <chrono>   // high_resolution_clock, duration, nanoseconds

namespace fmr { namespace perf {

using Float     = fmr::detail::Perf_float;
using Timepoint = fmr::detail::Perf_int  ;// ...One underlying type...
using Elapsed   = fmr::detail::Perf_int  ;// ...of these user types...
using Count     = fmr::detail::Perf_int  ;// ...used for easy struct packing.

// This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
// https://google.github.io/styleguide/cppguide.html#Aliases

static inline Timepoint get_now_ns () noexcept;

} }// end fmr::perf:: namespace
// Inline definitions =========================================================
namespace fmr {
  static inline perf::Timepoint perf::get_now_ns () noexcept {
    namespace sc = ::std::chrono;
    const sc::time_point <sc::high_resolution_clock, sc::nanoseconds> t
      = sc::high_resolution_clock::now();
    const sc::duration <perf::Timepoint, ::std::nano> d = t.time_since_epoch();
    return d.count();
  }
}// end fmr:: namespace

#undef FMR_DEBUG
//end FMR_HAS_PERF_HPP
#endif
