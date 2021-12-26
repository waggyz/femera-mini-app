#ifndef FMR_HAS_PERF_IPP
#define FMR_HAS_PERF_IPP

#include <chrono>   // high_resolution_clock, duration, nanoseconds

// Inline definitions
namespace fmr {

static inline fmr::Perf_int perf::get_now_ns () noexcept {
  namespace sc = ::std::chrono;
  const sc::time_point <sc::high_resolution_clock, sc::nanoseconds> t
    = sc::high_resolution_clock::now();
  const sc::duration <perf::Timepoint, ::std::nano> d = t.time_since_epoch();
  return d.count();
}

}// end fmr:: namespace
#undef FMR_DEBUG
//end FMR_HAS_PERF_IPP
#endif
