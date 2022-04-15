#ifndef FMR_HAS_PERF_METER_HPP
#define FMR_HAS_PERF_METER_HPP

#include "../perf.hpp"

#include <string>

namespace fmr { namespace perf {
  template <typename I, typename F>
  struct Meter {
  public:// methods
    std::string get_unit_name () noexcept;
    std::string set_unit_name (const std::string&) noexcept;
    //
    Timepoint start () noexcept;// returns start time (now)
    Timepoint reset () noexcept;// returns start time (reset does not restart)
    //
    // The remaining methods could throw integer or floating point exceptions.
    F add_busy_time_now ();// returns seconds of busy time added
    F add_idle_time_now ();// returns seconds of idle time added
    F add_count            // returns current unit_n total
      (I units=1, I flops=0, I read=0, I save=0);
    //
    F get_byte_n ();
    F get_arithmetic_intensity ();
    F get_ai ();// convenient synonym of get_arithmetic_intensity ()
    // Elapsed time
    F get_busy_s ();
    F get_idle_s ();
    F get_work_s ();
    Elapsed get_busy_ns ();
    Elapsed get_idle_ns ();
    Elapsed get_work_ns ();
    // Overall speed
    F get_unit_speed ();
    F get_flop_speed ();
    F get_data_speed ();
    F get_read_speed ();// was get_inp_speed ()
    F get_save_speed ();// was get_out_speed ()
    // Active speed
    F get_busy_unit_speed ();
    F get_busy_flop_speed ();
    F get_busy_data_speed ();
    F get_busy_read_speed ();
    F get_busy_save_speed ();
    // Constructors
    Meter (const std::string& unit_name) noexcept;
    Meter () =default;
  private:// All member variables are private.
    Timepoint tick    = ::fmr::perf::get_now_ns ();
    Elapsed   idle_ns = 0;
    Elapsed   busy_ns = 0;
    I         unit_n  = 0;
    I         flop_n  = 0;
    I         read_n  = 0;// bytes (was binp_n)
    I         save_n  = 0;// bytes (was bout_n)
    std::string unit_name = std::string("units");
  };
} }// end fmr::perf:: namespace

#include "Meter.ipp"

#undef FMR_DEBUG
//end FMR_HAS_PERF_METER_HPP
#endif
