#ifndef FMR_HAS_PERF_METER_IPP
#define FMR_HAS_PERF_METER_IPP

// Inline definitions
namespace fmr {
  inline perf::Meter::Meter (const std::string& units)
    noexcept : unit_name (units) {
  }
  inline std::string perf::Meter::get_unit_name ()
  noexcept {
    return this->unit_name;
  }
  inline std::string perf::Meter::set_unit_name (const std::string& name)
  noexcept {
    this->unit_name = name;
    return this->unit_name;
  }
  inline perf::Timepoint perf::Meter::start ()
  noexcept {
    this->tick = perf::get_now_ns ();
    return this->tick;
  }
  inline perf::Timepoint perf::Meter::reset ()
  noexcept {
    this->idle_ns = 0;
    this->busy_ns = 0;
    this->unit_n  = 0;
    this->flop_n  = 0;
    this->binp_n  = 0;
    this->bout_n  = 0;
    return this->start ();
  }
  inline fmr::Perf_float perf::Meter::add_busy_time_now () {
    const auto now = perf::get_now_ns ();
    this->busy_ns += now - this->tick;
    this->tick = now;
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (this->busy_ns);
  }
  inline fmr::Perf_float perf::Meter::add_idle_time_now () {
    const auto now = perf::get_now_ns ();
    this->idle_ns += now - this->tick;
    this->tick = now;
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (this->idle_ns);
  }
  inline fmr::Perf_float perf::Meter::add_count (const perf::Count flops,
      const perf::Count inp, const perf::Count out, const perf::Count units) {
    this->unit_n += units;
    this->flop_n += flops;
    this->binp_n += inp;
    this->bout_n += out;
    return fmr::Perf_float (this->unit_n);
  }
  inline fmr::Perf_float perf::Meter::get_byte_n () {
    return fmr::Perf_float (this->binp_n + this->bout_n);
  }
  inline fmr::Perf_float perf::Meter::get_arithmetic_intensity () {
    return fmr::Perf_float (this->flop_n) / this->get_byte_n ();
  }
  inline fmr::Perf_float perf::Meter::get_ai () {
    return perf::Meter::get_arithmetic_intensity ();
  }
  inline fmr::Perf_float perf::Meter::get_busy_s () {
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (this->busy_ns);
  }
  inline fmr::Perf_float perf::Meter::get_idle_s () {
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (this->idle_ns);
  }
  inline fmr::Perf_float perf::Meter::get_work_s () {
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (this->busy_ns + this->idle_ns);
  }
  #if 0
  //FIXME Are these useful?
  inline fmr::Perf_float perf::Meter::get_life_time_now () {
    return 1e-9 * fmr::Perf_float (perf::get_now_ns() - this->begin_at);
  }
  inline fmr::Perf_float perf::Meter::get_idle_time_now () {
    return 1e-9 * fmr::Perf_float (
      perf::get_now_ns() - this->begin_at - this->idle_ns - this->busy_ns);
  }
  inline fmr::Perf_float perf::Meter::get_life_flop_speed () {
    return fmr::Perf_float (this->flop_n) / this->get_life_time();
  }
  #endif
  // Overall speeds
  inline fmr::Perf_float perf::Meter::get_unit_speed () {
    return fmr::Perf_float (this->unit_n) / this->get_work_s ();
  }
  inline fmr::Perf_float perf::Meter::get_flop_speed () {
    return fmr::Perf_float (this->flop_n) / this->get_work_s ();
  }
  inline fmr::Perf_float perf::Meter::get_data_speed () {
    return fmr::Perf_float (this->binp_n + this->bout_n) / this->get_work_s();
  }
  inline fmr::Perf_float perf::Meter::get_inp_speed () {
    return fmr::Perf_float (this->binp_n) / this->get_work_s();
  }
  inline fmr::Perf_float perf::Meter::get_out_speed () {
    return fmr::Perf_float (this->bout_n) / this->get_work_s();
  }
  // Active speeds
  inline fmr::Perf_float perf::Meter::get_busy_unit_speed () {
    return fmr::Perf_float (this->unit_n) / this->get_busy_s ();
  }
  inline fmr::Perf_float perf::Meter::get_busy_flop_speed () {
    return fmr::Perf_float (this->flop_n) / this->get_busy_s ();
  }
  inline fmr::Perf_float perf::Meter::get_busy_data_speed () {
    return fmr::Perf_float (this->binp_n + this->bout_n) / this->get_busy_s();
  }
  inline fmr::Perf_float perf::Meter::get_busy_inp_speed () {
    return fmr::Perf_float (this->binp_n) / this->get_busy_s();
  }
  inline fmr::Perf_float perf::Meter::get_busy_out_speed () {
    return fmr::Perf_float (this->bout_n) / this->get_busy_s();
  }
}// end fmr:: namespace
#undef FMR_DEBUG
//end FMR_HAS_PERF_METER_IPP
#endif
