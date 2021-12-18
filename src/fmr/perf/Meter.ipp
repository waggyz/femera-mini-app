#ifndef FMR_HAS_PERF_METER_IPP
#define FMR_HAS_PERF_METER_IPP

// Inline definitions
namespace fmr {
  inline std::string perf::Meter::get_unit () noexcept {
    return this->unit_name;
  }
  inline std::string perf::Meter::set_unit (const std::string name) noexcept {
    this->unit_name = name;
    return this->unit_name;
  }
  inline perf::Timepoint perf::Meter::start () noexcept {
    this->tick = perf::get_now_ns ();
    return this->tick;
  }
  inline perf::Timepoint perf::Meter::reset () noexcept {
    this->idle_ns = 0;
    this->busy_ns = 0;
    this->unit_n  = 0;
    this->flop_n  = 0;
    this->binp_n  = 0;
    this->bout_n  = 0;
    return this->start ();
  }
  inline perf::Float perf::Meter::add_busy_time_now () {
    const auto now = perf::get_now_ns ();
    this->busy_ns += now - this->tick;
    this->tick = now;
    return perf::Float (1.0e-9) * perf::Float (this->busy_ns);
  }
  inline perf::Float perf::Meter::add_idle_time_now () {
    const auto now = perf::get_now_ns ();
    this->idle_ns += now - this->tick;
    this->tick = now;
    return perf::Float (1.0e-9) * perf::Float (this->idle_ns);
  }
  inline perf::Float perf::Meter::add_count (const perf::Count flops,
      const perf::Count inp, const perf::Count out, const perf::Count units) {
    this->unit_n += units;
    this->flop_n += flops;
    this->binp_n += inp;
    this->bout_n += out;
    return perf::Float (this->unit_n);
  }
  inline perf::Float perf::Meter::get_byte_n () {
    return perf::Float (this->binp_n + this->bout_n);
  }
  inline perf::Float perf::Meter::get_arithmetic_intensity () {
    return perf::Float (this->flop_n) / this->get_byte_n ();
  }
  inline perf::Float perf::Meter::get_ai () {
    return perf::Meter::get_arithmetic_intensity ();
  }
  inline perf::Float perf::Meter::get_busy_s () {
    return perf::Float (1.0e-9) * perf::Float (this->busy_ns);
  }
  inline perf::Float perf::Meter::get_idle_s () {
    return perf::Float (1.0e-9) * perf::Float (this->idle_ns);
  }
  inline perf::Float perf::Meter::get_work_s () {
    return perf::Float (1.0e-9) * perf::Float (this->busy_ns + this->idle_ns);
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
    return perf::Float (this->unit_n) / this->get_work_s ();
  }
  inline perf::Float perf::Meter::get_flop_speed () {
    return perf::Float (this->flop_n) / this->get_work_s ();
  }
  inline perf::Float perf::Meter::get_data_speed () {
    return perf::Float (this->binp_n + this->bout_n) / this->get_work_s();
  }
  inline perf::Float perf::Meter::get_inp_speed () {
    return perf::Float (this->binp_n) / this->get_work_s();
  }
  inline perf::Float perf::Meter::get_out_speed () {
    return perf::Float (this->bout_n) / this->get_work_s();
  }
  // Active speeds
  inline perf::Float perf::Meter::get_busy_unit_speed () {
    return perf::Float (this->unit_n) / this->get_busy_s ();
  }
  inline perf::Float perf::Meter::get_busy_flop_speed () {
    return perf::Float (this->flop_n) / this->get_busy_s ();
  }
  inline perf::Float perf::Meter::get_busy_data_speed () {
    return perf::Float (this->binp_n + this->bout_n) / this->get_busy_s();
  }
  inline perf::Float perf::Meter::get_busy_inp_speed () {
    return perf::Float (this->binp_n) / this->get_busy_s();
  }
  inline perf::Float perf::Meter::get_busy_out_speed () {
    return perf::Float (this->bout_n) / this->get_busy_s();
  }
}// end fmr:: namespace
#undef FMR_DEBUG
//end FMR_HAS_PERF_METER_IPP
#endif
