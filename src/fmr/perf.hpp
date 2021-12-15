#ifndef FMR_HAS_PERF_HPP
#define FMR_HAS_PERF_HPP

#include <string>

namespace fmr { namespace perf {
typedef unsigned long Int ;// This underlying type...
typedef Int Timepoint     ;// ...of all of these...
typedef Int Elapsed       ;// ...
typedef Int Count         ;// ...used for easy struct packing.
typedef float Float       ;

static inline Timepoint get_now_ns () noexcept;

struct Meter {
public:
  std::string unit_name ="units";
private:
  Timepoint tick    = get_now_ns ();
  Elapsed   idle_ns = 0;
  Elapsed   busy_ns = 0;
  Count     unit_n  = 0;
  Count     flop_n  = 0;
  Count     binp_n  = 0;
  Count     bout_n  = 0;
public:
  inline Timepoint start () noexcept;
  inline Elapsed add_busy_time_now (
    Count flops=0, Count inp=0, Count out=0, Count units=1);
  inline Elapsed add_idle_time_now ();
  inline Float get_busy_time ();
  inline Float get_idle_time ();
  inline Float get_work_time ();
  inline Float get_arithmetic_intensity ();
  // Overall speeds
  inline Float get_unit_speed ();
  inline Float get_flop_speed ();
  inline Float get_data_speed ();
  inline Float get_inp_speed ();
  inline Float get_out_speed ();
  // Active speeds
  inline Float get_busy_unit_speed ();
  inline Float get_busy_flop_speed ();
  inline Float get_busy_data_speed ();
  inline Float get_busy_inp_speed ();
  inline Float get_busy_out_speed ();
#if 0
  Timepoint begin   = get_now_ns ();//FIXME Useful?
  Timepoint start   = get_now_ns ();
  Elapsed   wait_ns = 0;
#endif
#if 0
  //FIXME Are these useful?
  inline Elapsed get_life_time_now ();
  inline Elapsed get_idle_time_now ();
  inline Float get_life_flop_speed ();
#endif
};
} }// end fmr::perf:: namespace
#undef FMR_DEBUG
//end FMR_HAS_PERF_HPP
#endif
