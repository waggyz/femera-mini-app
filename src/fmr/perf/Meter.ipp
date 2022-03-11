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
    this->read_n  = 0;
    this->save_n  = 0;
    return this->start ();
  }
  inline fmr::Perf_float perf::Meter::add_busy_time_now () {
    const auto now = perf::get_now_ns ();
    const auto last_busy = now - this->tick;
    this->tick = now;
    this->busy_ns += last_busy;
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (last_busy);
  }
  inline fmr::Perf_float perf::Meter::add_idle_time_now () {
    const auto now = perf::get_now_ns ();
    const auto last_idle = now - this->tick;
    this->tick = now;
    this->idle_ns += last_idle;
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (last_idle);
  }
  inline fmr::Perf_float perf::Meter::add_count (const perf::Count units,
    const perf::Count flops, const perf::Count read, const perf::Count save ) {
    this->unit_n += units;
    this->flop_n += flops;
    this->read_n += read;
    this->save_n += save;
    return fmr::Perf_float (this->unit_n);
  }
  inline fmr::Perf_float perf::Meter::get_byte_n () {
    return fmr::Perf_float (this->read_n + this->save_n);
  }
  inline fmr::Perf_float perf::Meter::get_arithmetic_intensity () {
    return fmr::Perf_float (this->flop_n) / this->get_byte_n ();
  }
  inline fmr::Perf_float perf::Meter::get_ai () {
    return fmr::Perf_float (this->flop_n) / this->get_byte_n ();
  }
  inline fmr::Perf_float perf::Meter::get_busy_s () {
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (this->busy_ns);
  }
  inline fmr::Perf_float perf::Meter::get_idle_s () {
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (this->idle_ns);
  }
  inline fmr::Perf_float perf::Meter::get_work_s () {
    return fmr::Perf_float (1.0e-9) * fmr::Perf_float (this->busy_ns + idle_ns);
  }
  inline fmr::perf::Elapsed perf::Meter::get_busy_ns () {
    return this->busy_ns;
  }
  inline fmr::perf::Elapsed perf::Meter::get_idle_ns () {
    return this->idle_ns;
  }
  inline fmr::perf::Elapsed perf::Meter::get_work_ns () {
    return this->busy_ns + idle_ns;
  }
  #if 0
  //TODO Are these useful?
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
    return fmr::Perf_float (this->read_n + this->save_n) / this->get_work_s();
  }
  inline fmr::Perf_float perf::Meter::get_read_speed () {
    return fmr::Perf_float (this->read_n) / this->get_work_s();
  }
  inline fmr::Perf_float perf::Meter::get_save_speed () {
    return fmr::Perf_float (this->save_n) / this->get_work_s();
  }
  // Active speeds
  inline fmr::Perf_float perf::Meter::get_busy_unit_speed () {
    return fmr::Perf_float (this->unit_n) / this->get_busy_s ();
  }
  inline fmr::Perf_float perf::Meter::get_busy_flop_speed () {
    return fmr::Perf_float (this->flop_n) / this->get_busy_s ();
  }
  inline fmr::Perf_float perf::Meter::get_busy_data_speed () {
    return fmr::Perf_float (this->read_n + this->save_n) / this->get_busy_s();
  }
  inline fmr::Perf_float perf::Meter::get_busy_read_speed () {
    return fmr::Perf_float (this->read_n) / this->get_busy_s();
  }
  inline fmr::Perf_float perf::Meter::get_busy_save_speed () {
    return fmr::Perf_float (this->save_n) / this->get_busy_s();
  }
}// end fmr:: namespace
#undef FMR_DEBUG
//end FMR_HAS_PERF_METER_IPP
#endif
