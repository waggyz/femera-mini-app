#include "perf.cpp"

#include <chrono>   // high_resolution_clock, duration_cast

namespace fmr {
  static inline perf::Timepoint perf::get_now_ns () noexcept {
    namespace sc = ::std::chrono;
    const sc::time_point <sc::high_resolution_clock, sc::nanoseconds> t
      = sc::high_resolution_clock::now();
    const sc::duration <Timepoint, ::std::nano> d
      = t.time_since_epoch();
    return d.count();
  }
  inline perf::Timepoint perf::start () noexcept {
    this.tick = perf::get_now_ns ();
    return this.tick;
  }
  inline perf::Elapsed perf::add_busy_time_now (perf::Count flops=0,
      perf::Count inp=0, perf::Count out=0, perf::Count units=1) {
    const auto now = perf::get_now_ns ();
    this.busy_ns += now - this.tick;
    this.tick = now;
    this.unit_n += units;
    this.flop_n += flops;
    this.binp_n += inp;
    this.bout_n += out;
    return this.busy_ns;
  }
  inline perf::Elapsed perf::add_idle_time_now () {
    const auto now = perf::get_now_ns ();
    this.idle_ns += now - this.tick;
    this.tick = now;
    return this.idle_ns;
  }
  inline perf::Float perf::get_busy_time () {
    return 1e-9 * perf::Float (this.busy_ns);
  }
  inline perf::Float perf::get_idle_time () {
    return 1e-9 * perf::Float (this.idle_ns);
  }
  inline perf::Float perf::get_work_time () {
    return 1e-9 * perf::Float (this.busy_ns + this.idle_ns);
  }
  #if 0
  //FIXME Are these useful?
  inline perf::Float perf::get_life_time_now () {
    return 1e-9 * perf::Float (perf::get_now_ns() - this.begin_at);
  }
  inline perf::Float perf::get_idle_time_now () {
    return 1e-9 * perf::Float (
      perf::get_now_ns() - this.begin_at - this.idle_ns - this.busy_ns);
  }
  inline perf::Float perf::get_life_flop_speed () {
    return perf::Float (this.flop_n) / this.get_life_time();
  }
  #endif
  inline perf::Float perf::get_arithmetic_intensity () {
    return perf::Float (this.flop_n) / perf::Float (this.binp_n + this.bout_n);
  }
  // Overall speeds
  inline perf::Float perf::get_unit_speed () {
    return perf::Float (this.unit_n) / this.get_work_time ();
  }
  inline perf::Float perf::get_flop_speed () {
    return perf::Float (this.flop_n) / this.get_work_time ();
  }
  inline perf::Float perf::get_data_speed () {
    return perf::Float (this.binp_n + this.bout_n) / this.get_work_time();
  }
  inline perf::Float perf::get_inp_speed () {
    return perf::Float (this.binp_n) / this.get_work_time();
  }
  inline perf::Float perf::get_out_speed () {
    return perf::Float (this.bout_n) / this.get_work_time();
  }
  // Active speeds
  inline perf::Float perf::get_busy_unit_speed () {
    return perf::Float (this.unit_n) / this.get_busy_time ();
  }
  inline perf::Float perf::get_busy_flop_speed () {
    return perf::Float (this.flop_n) / this.get_busy_time ();
  }
  inline perf::Float perf::get_busy_data_speed () {
    return perf::Float (this.binp_n + this.bout_n) / this.get_busy_time();
  }
  inline perf::Float perf::get_busy_inp_speed () {
    return perf::Float (this.binp_n) / this.get_busy_time();
  }
  inline perf::Float get_busy_out_speed () {
    return perf::Float (this.bout_n) / this.get_busy_time();
  }
}// end fmr:: namespace
