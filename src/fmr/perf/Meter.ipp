#ifndef FMR_HAS_PERF_METER_IPP
#define FMR_HAS_PERF_METER_IPP

#if 0
// Change to 1 if atomic operators needed
#define THIS_PRAGMA_OMP(x) FMR_PRAGMA_OMP(x)
#else
#define THIS_PRAGMA_OMP(x) // pragma omp not used here
#endif
// Inline definitions
namespace fmr {
  template <typename I, typename F> inline
  perf::Meter<I,F>::Meter (const std::string& units)
    noexcept : unit_name (units) {
  }
  template <typename I, typename F> inline
  std::string perf::Meter<I,F>::get_unit_name ()
  noexcept {
    return this->unit_name;
  }
  template <typename I, typename F> inline
  std::string perf::Meter<I,F>::set_unit_name (const std::string& name)
  noexcept {
    this->unit_name = name;
    return this->unit_name;
  }
  template <typename I, typename F> inline
  perf::Timepoint perf::Meter<I,F>::start ()
  noexcept {
    THIS_PRAGMA_OMP(omp atomic write)
    this->tick = perf::get_now_ns ();
    return this->tick;
  }
  template <typename I, typename F> inline
  perf::Timepoint perf::Meter<I,F>::reset ()
  noexcept {
    THIS_PRAGMA_OMP(omp atomic write)
    this->idle_ns = 0;
    THIS_PRAGMA_OMP(omp atomic write)
    this->busy_ns = 0;
    THIS_PRAGMA_OMP(omp atomic write)
    this->unit_n  = 0;
    THIS_PRAGMA_OMP(omp atomic write)
    this->flop_n  = 0;
    THIS_PRAGMA_OMP(omp atomic write)
    this->read_n  = 0;
    THIS_PRAGMA_OMP(omp atomic write)
    this->save_n  = 0;
    return this->start ();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::add_busy_time_now () {
    const fmr::perf::Count now = perf::get_now_ns ();
    const fmr::perf::Count last_busy = (now > this->tick) ? now - tick : 0;
    THIS_PRAGMA_OMP(omp atomic write)
    this->tick = now;
    THIS_PRAGMA_OMP(omp atomic update)
    this->busy_ns += last_busy;
    return F (1.0e-9) * F (last_busy);
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::add_idle_time_now () {
    const fmr::perf::Count now = perf::get_now_ns ();
    const fmr::perf::Count last_idle = (now > this->tick) ? now - tick : 0;
    THIS_PRAGMA_OMP(omp atomic write)
    this->tick = now;
    THIS_PRAGMA_OMP(omp atomic update)
    this->idle_ns += last_idle;
    return F (1.0e-9) * F (last_idle);
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::add_count
    (const I units, const I flops, const I read, const I save ) {
    THIS_PRAGMA_OMP(omp atomic update)
    this->unit_n += units;
    THIS_PRAGMA_OMP(omp atomic update)
    this->flop_n += flops;
    THIS_PRAGMA_OMP(omp atomic update)
    this->read_n += read;
    THIS_PRAGMA_OMP(omp atomic update)
    this->save_n += save;
    return F (this->unit_n);
  }
  // The following return integer type (I).
  template <typename I, typename F> inline
  I perf::Meter<I,F>::get_unit_n() {
    return this->unit_n;
  }
  template <typename I, typename F> inline
  I perf::Meter<I,F>::get_flop_n () {
    return this->flop_n;
  }
  template <typename I, typename F> inline
  I perf::Meter<I,F>::get_read_n () {
    return this->read_n;
  }
  template <typename I, typename F> inline
  I perf::Meter<I,F>::get_save_n () {
    return this->save_n;
  }
  template <typename I, typename F> inline
  I perf::Meter<I,F>::get_byte_n () {
    return this->read_n + this->save_n;
  }
  // The following return floating point type (F).
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_unit () {
    return F (this->unit_n);
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_flop () {
    return F (this->flop_n);
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_read () {
    return F (this->read_n);
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_save () {
    return F (this->save_n);
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_byte () {
    return F (this->read_n + this->save_n);
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_arithmetic_intensity () {
    return F (this->flop_n) / this->get_byte ();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_ai () {
    return F (this->flop_n) / this->get_byte ();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_busy_s () {
    return F (1.0e-9) * F (this->busy_ns);
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_idle_s () {
    return F (1.0e-9) * F (this->idle_ns);
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_work_s () {
    this->add_idle_time_now ();
    return F (1.0e-9) * F (this->busy_ns + idle_ns);
  }
  template <typename I, typename F> inline
  fmr::perf::Elapsed perf::Meter<I,F>::get_busy_ns () {
    return this->busy_ns;
  }
  template <typename I, typename F> inline
  fmr::perf::Elapsed perf::Meter<I,F>::get_idle_ns () {
    return this->idle_ns;
  }
  template <typename I, typename F> inline
  fmr::perf::Elapsed perf::Meter<I,F>::get_work_ns () {
    return this->busy_ns + idle_ns;
  }
  #if 0
  //TODO Are these useful? inline
  F perf::Meter<I,F>::get_life_time_now () {
    return 1e-9 * F (perf::get_now_ns() - this->begin_at);
  } inline
  F perf::Meter<I,F>::get_idle_time_now () {
    return 1e-9 * F (
      perf::get_now_ns() - this->begin_at - this->idle_ns - this->busy_ns);
  } inline
  F perf::Meter<I,F>::get_life_flop_speed () {
    return F (this->flop_n) / this->get_life_time();
  }
  #endif
  // Overall speeds
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_unit_speed () {
    return F (this->unit_n) / this->get_work_s ();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_flop_speed () {
    return F (this->flop_n) / this->get_work_s ();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_data_speed () {
    return F (this->read_n + this->save_n) / this->get_work_s();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_read_speed () {
    return F (this->read_n) / this->get_work_s();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_save_speed () {
    return F (this->save_n) / this->get_work_s();
  }
  // Active speeds
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_busy_unit_speed () {
    return F (this->unit_n) / this->get_busy_s ();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_busy_flop_speed () {
    return F (this->flop_n) / this->get_busy_s ();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_busy_data_speed () {
    return F (this->read_n + this->save_n) / this->get_busy_s();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_busy_read_speed () {
    return F (this->read_n) / this->get_busy_s();
  }
  template <typename I, typename F> inline
  F perf::Meter<I,F>::get_busy_save_speed () {
    return F (this->save_n) / this->get_busy_s();
  }
}// end fmr:: namespace
#undef FMR_DEBUG
#undef THIS_PRAGMA_OMP
//end FMR_HAS_PERF_METER_IPP
#endif
