#ifndef FMR_HAS_PERF_METER_HPP
#define FMR_HAS_PERF_METER_HPP

#include "../perf.hpp"

#include <string>

namespace fmr { namespace perf {

struct Meter {
public:// methods
  inline std::string get_unit () noexcept;
  inline std::string set_unit (const std::string name) noexcept;
  //
  inline Timepoint start () noexcept;
  inline Timepoint reset () noexcept;
  // The remaining methods could throw integer or floating point exceptions.
  //
  inline Float add_busy_time_now ();
  inline Float add_idle_time_now ();
  inline Float add_count (const Count flops=0,
    const Count inp=0, const Count out=0, const Count units=1);
  //
  inline Float get_byte_n ();
  inline Float get_arithmetic_intensity ();
  inline Float get_ai ();// convenient synonym of get_arithmetic_intensity ()
  // Elapsed time
  inline Float get_busy_s ();
  inline Float get_idle_s ();
  inline Float get_work_s ();
  // Overall speed
  inline Float get_unit_speed ();
  inline Float get_flop_speed ();
  inline Float get_data_speed ();
  inline Float get_inp_speed ();
  inline Float get_out_speed ();
  // Active speed
  inline Float get_busy_unit_speed ();
  inline Float get_busy_flop_speed ();
  inline Float get_busy_data_speed ();
  inline Float get_busy_inp_speed ();
  inline Float get_busy_out_speed ();
private:
  Timepoint tick    = get_now_ns ();
  Elapsed   idle_ns = 0;
  Elapsed   busy_ns = 0;
  Count     unit_n  = 0;
  Count     flop_n  = 0;
  Count     binp_n  = 0;
  Count     bout_n  = 0;
  std::string unit_name ="units";
#if 0
private:
  Timepoint begin   = get_now_ns ();//FIXME Useful?
  Timepoint start   = get_now_ns ();
  Elapsed   wait_ns = 0;
#endif
#if 0
public:
  //FIXME Are these useful?
  inline Elapsed get_life_time_now ();
  inline Elapsed get_idle_time_now ();
  inline Float get_life_flop_speed ();
#endif
};
} }// end fmr::perf:: namespace

#include "Meter.ipp"

#undef FMR_DEBUG
//end FMR_HAS_PERF_METER_HPP
#endif
