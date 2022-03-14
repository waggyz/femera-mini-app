#ifndef FMR_HAS_PERF_METER_HPP
#define FMR_HAS_PERF_METER_HPP

#include "../perf.hpp"

#include <string>

namespace fmr { namespace perf {
  template <typename T>
  struct Meter {
  private:
    using Float = fmr::Perf_float;
  public:// methods
    std::string get_unit_name () noexcept;
    std::string set_unit_name (const std::string&) noexcept;
    //
    Timepoint start () noexcept;// returns start time (now)
    Timepoint reset () noexcept;// returns start time (reseet does not restart)
    //
    // The remaining methods could throw integer or floating point exceptions.
    Float add_busy_time_now ();// returns seconds of busy time added
    Float add_idle_time_now ();// returns seconds of idle time added
    Float add_count            // returns current unit_n total
      (T units=1, T flops=0, T read=0, T save=0);
    //
    Float get_byte_n ();
    Float get_arithmetic_intensity ();
    Float get_ai ();// convenient synonym of get_arithmetic_intensity ()
    // Elapsed time
    Float get_busy_s ();
    Float get_idle_s ();
    Float get_work_s ();
    Elapsed get_busy_ns ();
    Elapsed get_idle_ns ();
    Elapsed get_work_ns ();
    // Overall speed
    Float get_unit_speed ();
    Float get_flop_speed ();
    Float get_data_speed ();
    Float get_read_speed ();// was get_inp_speed ()
    Float get_save_speed ();// was get_out_speed ()
    // Active speed
    Float get_busy_unit_speed ();
    Float get_busy_flop_speed ();
    Float get_busy_data_speed ();
    Float get_busy_read_speed ();
    Float get_busy_save_speed ();
    // Constructors
    Meter (const std::string& unit_name) noexcept;
    Meter () =default;
  private:// All member variables are private.
    Timepoint tick    = ::fmr::perf::get_now_ns ();
    Elapsed   idle_ns = 0;
    Elapsed   busy_ns = 0;
    T         unit_n  = 0;
    T         flop_n  = 0;
    T         read_n  = 0;// bytes (was binp_n)
    T         save_n  = 0;// bytes (was bout_n)
    std::string unit_name = std::string("units");
  };
} }// end fmr::perf:: namespace

#include "Meter.ipp"

#undef FMR_DEBUG
//end FMR_HAS_PERF_METER_HPP
#endif
