#ifndef FMR_HAS_PERF_HPP
#define FMR_HAS_PERF_HPP

#include <string>
#include <chrono>   // high_resolution_clock, duration, nanoseconds

namespace fmr { namespace perf {
typedef float Float       ;
typedef unsigned long Int ;// This underlying type...
typedef Int     Timepoint ;// ...of all of these...
typedef Int       Elapsed ;// ...
typedef Int         Count ;// ...used for easy struct packing.

static inline Timepoint get_now_ns () noexcept;

struct Meter {
public:// variables
  std::string unit_name ="units";
public:// methods
  inline Timepoint start () noexcept;
  // The remaining methods could throw integer or floating point exceptions.
  inline Elapsed add_busy_time_now (const Count flops=0,
    const Count inp=0, const Count out=0, const Count units=1);
  inline Elapsed add_idle_time_now ();
  //
  inline Float get_byte_n ();
  inline Float get_arithmetic_intensity ();
  inline Float get_busy_time ();
  inline Float get_idle_time ();
  inline Float get_work_time ();
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
private:
  Timepoint tick    = get_now_ns ();
  Elapsed   idle_ns = 0;
  Elapsed   busy_ns = 0;
  Count     unit_n  = 0;
  Count     flop_n  = 0;
  Count     binp_n  = 0;
  Count     bout_n  = 0;
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
namespace fmr {
  static inline perf::Timepoint perf::get_now_ns () noexcept {
    namespace sc = ::std::chrono;
    const sc::time_point <sc::high_resolution_clock, sc::nanoseconds> t
      = sc::high_resolution_clock::now();
    const sc::duration <perf::Timepoint, ::std::nano> d = t.time_since_epoch();
    return d.count();
  }
  inline perf::Timepoint perf::Meter::start () noexcept {
    this->tick = perf::get_now_ns ();
    return this->tick;
  }
  inline perf::Elapsed perf::Meter::add_busy_time_now (const perf::Count flops,
      const perf::Count inp, const perf::Count out, const perf::Count units) {
    const auto now = perf::get_now_ns ();
    this->busy_ns += now - this->tick;
    this->tick = now;
    this->unit_n += units;
    this->flop_n += flops;
    this->binp_n += inp;
    this->bout_n += out;
    return this->busy_ns;
  }
  inline perf::Elapsed perf::Meter::add_idle_time_now () {
    const auto now = perf::get_now_ns ();
    this->idle_ns += now - this->tick;
    this->tick = now;
    return this->idle_ns;
  }
  inline perf::Float perf::Meter::get_byte_n () {
    return perf::Float (this->binp_n + this->bout_n);
  }
  inline perf::Float perf::Meter::get_arithmetic_intensity () {
    return perf::Float (this->flop_n) / this->get_byte_n ();
  }
  inline perf::Float perf::Meter::get_busy_time () {
    return perf::Float (1e-9) * perf::Float (this->busy_ns);
  }
  inline perf::Float perf::Meter::get_idle_time () {
    return perf::Float (1e-9) * perf::Float (this->idle_ns);
  }
  inline perf::Float perf::Meter::get_work_time () {
    return perf::Float (1e-9) * perf::Float (this->busy_ns + this->idle_ns);
  }
  #if 0
  //FIXME Are these useful?
  inline perf::Float perf::Meter::get_life_time_now () {
    return 1e-9 * perf::Float (perf::get_now_ns() - this->begin_at);
  }
  inline perf::Float perf::Meter::get_idle_time_now () {
    return 1e-9 * perf::Float (
      perf::get_now_ns() - this->begin_at - this->idle_ns - this->busy_ns);
  }
  inline perf::Float perf::Meter::get_life_flop_speed () {
    return perf::Float (this->flop_n) / this->get_life_time();
  }
  #endif
  // Overall speeds
  inline perf::Float perf::Meter::get_unit_speed () {
    return perf::Float (this->unit_n) / this->get_work_time ();
  }
  inline perf::Float perf::Meter::get_flop_speed () {
    return perf::Float (this->flop_n) / this->get_work_time ();
  }
  inline perf::Float perf::Meter::get_data_speed () {
    return perf::Float (this->binp_n + this->bout_n) / this->get_work_time();
  }
  inline perf::Float perf::Meter::get_inp_speed () {
    return perf::Float (this->binp_n) / this->get_work_time();
  }
  inline perf::Float perf::Meter::get_out_speed () {
    return perf::Float (this->bout_n) / this->get_work_time();
  }
  // Active speeds
  inline perf::Float perf::Meter::get_busy_unit_speed () {
    return perf::Float (this->unit_n) / this->get_busy_time ();
  }
  inline perf::Float perf::Meter::get_busy_flop_speed () {
    return perf::Float (this->flop_n) / this->get_busy_time ();
  }
  inline perf::Float perf::Meter::get_busy_data_speed () {
    return perf::Float (this->binp_n + this->bout_n) / this->get_busy_time();
  }
  inline perf::Float perf::Meter::get_busy_inp_speed () {
    return perf::Float (this->binp_n) / this->get_busy_time();
  }
  inline perf::Float perf::Meter::get_busy_out_speed () {
    return perf::Float (this->bout_n) / this->get_busy_time();
  }
}// end fmr:: namespace

#undef FMR_DEBUG
//end FMR_HAS_PERF_HPP
#endif
