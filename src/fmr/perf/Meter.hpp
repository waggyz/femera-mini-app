#ifndef FMR_HAS_PERF_METER_HPP
#define FMR_HAS_PERF_METER_HPP

#include "../perf.hpp"

#include <string>

namespace fmr { namespace perf {
  struct Meter {
  private:
    using Float = fmr::Perf_float;
  public:// methods
    std::string get_unit_name () noexcept;
    std::string set_unit_name (std::string) noexcept;
    //
    Timepoint start () noexcept;
    Timepoint reset () noexcept;
    //
    // The remaining methods could throw integer or floating point exceptions.
    Float add_busy_time_now ();
    Float add_idle_time_now ();
    Float add_count (Count flops=0, Count inp=0, Count out=0, Count units=1);
    //
    Float get_byte_n ();
    Float get_arithmetic_intensity ();
    Float get_ai ();// convenient synonym of get_arithmetic_intensity ()
    // Elapsed time
    Float get_busy_s ();
    Float get_idle_s ();
    Float get_work_s ();
    // Overall speed
    Float get_unit_speed ();
    Float get_flop_speed ();
    Float get_data_speed ();
    Float get_inp_speed ();
    Float get_out_speed ();
    // Active speed
    Float get_busy_unit_speed ();
    Float get_busy_flop_speed ();
    Float get_busy_data_speed ();
    Float get_busy_inp_speed ();
    Float get_busy_out_speed ();
    // Constructors
    Meter (std::string unit_name) noexcept;
    Meter () =default;
  private:
    Timepoint tick    = get_now_ns ();
    Elapsed   idle_ns = 0;
    Elapsed   busy_ns = 0;
    Count     unit_n  = 0;
    Count     flop_n  = 0;
    Count     binp_n  = 0;
    Count     bout_n  = 0;
    std::string unit_name ="units";
  };
} }// end fmr::perf:: namespace

#include "Meter.ipp"

#undef FMR_DEBUG
//end FMR_HAS_PERF_METER_HPP
#endif
