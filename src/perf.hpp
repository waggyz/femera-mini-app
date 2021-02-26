#ifndef FMR_API_PERF_HPP
#define FMR_API_PERF_HPP
/** */

#include <chrono>   // high_resolution_clock, duration_cast
#include <string>
#include <cmath>    // pow()
#include <cstring>  // strlen()
//#include <functional> //std::function
#include <type_traits>  // enable_if

// public API ----------------------------------------------------------------
namespace fmr { namespace perf {

typedef      long Perf_int  ;// This underlying type...
typedef  Perf_int Timepoint ;// ...of all of these...
typedef  Perf_int Elapsed   ;// ...
typedef  Perf_int Count     ;// ...used for easy struct packing.

typedef long long Accum     ;// sizeof(Accum) > sizeof(Perf_int)
typedef     float Float     ;

static inline Timepoint get_now_ns () noexcept;
#if 0
struct Timer {
  Timepoint start        = get_now_ns ();
  Timepoint click        = start;
  Elapsed   last_idle_ns = 0;
  Elapsed   last_busy_ns = 0;
  Elapsed        busy_ns = 0;// time doing work
};
#endif
struct Meter {
  //TODO add work_ns to differentiate busy overhead from useful work time?
  Timepoint start        = get_now_ns ();
  Timepoint click        = start;
  Elapsed   last_busy_ns = 0;// Valid after timer_pause()
  Elapsed   last_idle_ns = 0;// Valid after timer_resume(), _pause()
  Elapsed   busy_ns      = 0;
  Count     flops        = 0;
  Count     bytes        = 0;
  Count     count        = 0;// generic counter
  //TODO overload count for val)array, struct? counts.
  //std::string units    = "unit";//TODO
};
#if 0
struct BWmeter {
  Timepoint start        = get_now_ns ();
  Timepoint click        = start;
  Elapsed   last_busy_ns = 0;// Valid after timer_pause()
  Elapsed   last_idle_ns = 0;// Valid after timer_resume(), _pause()
  Elapsed   busy_ns      = 0;//TODO read_ns, write_ns
  Count     flops        = 0;
  Count     bytes_read   = 0;
  Count     bytes_write  = 0;//TODO overload for (val)array, struct? counts.
};
#endif
//TODO implement for Timer, Meter, and BWmeter
//
// The timer_start()    Start by counting idle time.
// Put timer_resume()   Place immediately before starting work.
// Put timer_pause(...) Place immediately after work is done.
static inline Timepoint timer_start  (Meter* meter);
static inline Timepoint timer_resume (Meter* timer);
static inline Timepoint timer_pause  (Meter* timer);
static inline Timepoint timer_pause  (Meter* meter,
  const Count accum);
static inline Timepoint timer_pause  (Meter* meter,
  const Count accum, const Count flops, const Count bytes);
static inline Timepoint timer_stop  (Meter* timer);

extern inline Elapsed timer_busy_elapsed  (const Meter timer);
extern inline Elapsed timer_idle_elapsed  (const Meter timer);
extern inline Elapsed timer_total_elapsed (const Meter timer);

extern inline Float timer_busy_ns  (const Meter timer);
extern inline Float timer_idle_ns  (const Meter timer);
extern inline Float timer_total_ns (const Meter timer);

extern inline Float overall_speed  (const Meter meter);
extern inline Float busy_speed     (const Meter meter);
} }//end fmr::perf namespace

namespace fmr { namespace perf {// wrappers to time single funtion calls
//NOTE Need full specialization to wrap overloaded member functions?

template< typename R, typename T, class F,
  class = typename std::enable_if<std::is_void<R>::value>::type >
void time_activity (T* timer, F fn);// function w/out args, returns void

template< typename R, typename T, class F, typename... A,
  class = typename std::enable_if<std::is_void<R>::value>::type >
void time_activity (T* timer, F fn, A... args);// fn w/args, returns void

template< typename R, typename T, class F,
  class = typename std::enable_if<!std::is_void<R>::value>::type >
auto time_activity (T* timer, F fn)-> R;// fn w/out args, returns non-void

template< typename R, typename T, class F, typename... A,
  class = typename std::enable_if<!std::is_void<R>::value>::type >
auto time_activity (T* timer, F fn, A... args)-> R;// fn w/args, returns R
} }//end fmr::perf namespace

namespace fmr { namespace perf {// Format measurements for printing.
template<typename V>
std::string format_units (const V val, const std::string unit);

template<typename V>
std::string format_count (const V val, const std::string unit);

template<typename V>
std::string format_speed (const V val, std::string unit);

static inline std::string format_time_units (Elapsed ns);
} }//end fmr::perf namespace

// not meant for public use ==================================================
namespace fmr { namespace perf { namespace detail {
template<typename V>
std::string format_units_sigfigs (const V val, std::string unit,
  const int sigfigs);// minimum sigfigs for val < 1000: 1 or 2
} } }//end fmr::perf::detail:: namespace

// header-only inline implementation -----------------------------------------
namespace fmr { namespace perf {

static inline Timepoint get_now_ns () noexcept{
  namespace sc = ::std::chrono;
  const sc::time_point <sc::high_resolution_clock, sc::nanoseconds> t
    = sc::high_resolution_clock::now();
  const sc::duration <Timepoint, ::std::nano> d
    = t.time_since_epoch();
  return d.count();
}
//NOTE Looking at object files indicates that using these functions directly
// inlines chrono calls, but a template form does not.
static inline Timepoint timer_start (Meter* meter){
  meter->last_busy_ns   = Elapsed(0);
  meter->last_idle_ns   = Elapsed(0);
  meter->busy_ns        = Elapsed(0);
  meter->count          = Count(0);
  meter->flops          = Count(0);
  meter->bytes          = Count(0);
  const auto now_ns     = get_now_ns();
  meter->start          = Timepoint (now_ns);
  meter->click          = Timepoint (now_ns);
  return now_ns;
}
static inline Timepoint timer_resume (Meter* timer){
  // Put timer_resume() immediately before starting work.
  const auto now_ns     = get_now_ns ();
  timer->last_idle_ns   = Elapsed   (now_ns - timer->click);
  timer->click          = Timepoint (now_ns);
  return now_ns;
}
static inline Timepoint timer_pause (Meter* timer){
  // Put timer_pause() immediately after work is done.
  const auto now_ns     = get_now_ns();
  timer->last_busy_ns   = Elapsed (now_ns - timer->click);
  timer->busy_ns       += Elapsed (now_ns - timer->click);
  timer->click          = Timepoint (now_ns );
  return now_ns;
}
static inline Timepoint timer_pause (Meter* meter,
  const Count accum){
  // Put timer_pause() immediately after work is done.
  const auto now_ns     = get_now_ns();
  meter->last_busy_ns   = Elapsed (now_ns - meter->click);
  meter->busy_ns       += Elapsed (now_ns - meter->click);
  meter->count         += accum;
  meter->click          = Timepoint (now_ns);
  return now_ns;
}
static inline Timepoint timer_pause (Meter* meter,
  const Count accum, const Count f, const Count b){
  // Put timer_pause() immediately after work is done.
  const auto now_ns     = get_now_ns();
  meter->last_busy_ns   = Elapsed (now_ns - meter->click);
  meter->busy_ns       += Elapsed (now_ns - meter->click);
  meter->count         += accum;
  meter->flops         += f;
  meter->bytes         += b;
  meter->click          = Timepoint (now_ns);
  return now_ns;
}
static inline Timepoint timer_stop (Meter* timer){
  const auto now_ns     = get_now_ns ();
  timer->click          = Timepoint (now_ns);
  return now_ns;
}
Elapsed timer_busy_elapsed  (const Meter timer){
  return timer.busy_ns;
}
Elapsed timer_idle_elapsed  (const Meter timer){
  return timer.click - timer.start - timer.busy_ns;
}
Elapsed timer_total_elapsed  (const Meter timer){
  return timer.click - timer.start;
}
Float timer_busy_ns (const Meter timer){
  return Float (timer.busy_ns);
}
Float timer_idle_ns (const Meter timer){
  return Float (timer.click - timer.start - timer.busy_ns);
}
Float timer_total_ns (const Meter timer){
  return Float (timer.click - timer.start);
}
Float overall_speed (const Meter meter){
  return Float (1e9) * Float(meter.count)
    / Float (meter.click - meter.start);
}
Float busy_speed (const Meter meter){
  return Float (1e9) * Float(meter.count)
    / Float (meter.busy_ns);
}
template< typename R, typename T, class F,
class = typename std::enable_if<std::is_void<R>::value>::type >
void time_activity (T* timer, F fn){
  timer_resume (timer);
  fn           ();
  timer_pause  (timer);
}
template< typename R, typename T, class F, typename... A,
class = typename std::enable_if<std::is_void<R>::value>::type >
void time_activity (T* timer, F fn, A... args) {
  timer_resume (timer);
  fn           (args...);
  timer_pause  (timer);
}
template< typename R, typename T, class F,
class = typename std::enable_if<!std::is_void<R>::value>::type >
auto time_activity (T* timer, F fn)-> R {
  timer_resume (timer);
  R ret = fn   ();
  timer_pause  (timer);
  return ret;
}
template< typename R, typename T, class F, typename... A,
class = typename std::enable_if<!std::is_void<R>::value>::type >
auto time_activity (T* timer, F fn, A... args)-> R {
  timer_resume (timer);
  R ret = fn   (args...);
  timer_pause  (timer);
  return ret;
}
template<typename V>
std::string detail::format_units_sigfigs (const V val, std::string unit,
    const int sigfigs) {
  int log1000 = 0; double v = double(val);
  if (unit.size()>8) { unit = unit.substr (0,8); }
  const double threshold = std::pow (10.0, double(sigfigs-1));
  if (std::abs(v) > 1e-24) {
    log1000 = int(std::log10 (std::abs(v)))/3 - (std::abs(v)<1 ? 1:0);
    v = double(val) * std::pow (10.0, double(-3 * log1000));
    if (v < threshold) { v *= 1000.0; log1000 -= 1; }
  }
  const uint i = log1000+6;
  const char prefix[]="afpnum kMGTPE"; std::string pre = "?";
  if (i>=0 && i<std::strlen(prefix)) { pre = prefix[i]; }
  if (pre=="u") { pre="\u00b5"; }// micro u00b5 (Greek mu u03bc preferred)
  std::vector<char> buf(16,0);
  std::snprintf (&buf[0],15,"%4.0f %s%s", v, pre.c_str(), unit.c_str());
  return std::string(&buf[0]);
}
template<typename V>
std::string format_units (const V val, const std::string unit) {
  return detail::format_units_sigfigs (val, unit, 2);
}
template<typename V>
std::string format_count (const V val, const std::string unit) {// val >=1
  const int threshhold = (double(std::abs(val)) < 1000.0) ? 1:2;
  return detail::format_units_sigfigs (val, unit, threshhold);
}
template<typename V>
std::string format_speed (const V val, std::string unit) {
  if (unit.size() > 8) { unit = unit.substr (0,8); }
  double v = double(val);
  std::string time_str ="s"; std::string pre = " ";
  const bool do_invert = std::abs (v) < 1.0;
  if (do_invert) {// format as time/unit
    v = 1.0 / v;
    if (v > 600.0) { v /= 60.0; time_str="m"; }
    if (v > 600.0) { v /= 60.0; time_str="h";
      if (v > 240.0) { v /= 24.0; time_str="d"; }
  } }else{// format as unit/time
    int log1000 = 0;
    if (std::abs (v) > 1e-24) {
      log1000 = int(std::log10 (std::abs(v)))/3 - (std::abs(v)<1 ? 1:0);
      v = double(val) * std::pow (10.0, double(-3 * log1000));
      if (v<10.0) { v *= 1000.0; log1000 -= 1; }
    }
    const uint i = log1000 +6; pre = "?";
    const char prefix[]="afpnum kMGTPE";
    if (i>=0 && i<std::strlen(prefix)) { pre = prefix[i]; }
    if (pre=="u") { pre="\u00b5"; }//unicode micro \u00b5 (prefe Greek mu u03bc)
  }
  std::vector<char> buf(16,0);
  if (do_invert) {
    std::snprintf (&buf[0],15,"%4.0f %s%s/%s", v, pre.c_str(),time_str.c_str(),
      unit.c_str());
  }else{
    std::snprintf (&buf[0],15,"%4.0f %s%s/s", v, pre.c_str(),unit.c_str());
  }
  return std::string(&buf[0]);
}
static inline std::string format_time_units (Elapsed ns){
  const double sec = double(ns)*1e-9;
  std::string u = "s"; double t=sec;
  if      (sec>24.0*60.0*60.0){ t=sec/(24.0*60.0*60.0); u="d"; }
  else if (sec>     60.0*60.0){ t=sec/(     60.0*60.0); u="h"; }
  else if (sec>          60.0){ t=sec/           60.0 ; u="m"; }
  if (u=="s"){ return format_units (sec,"s"); }
  std::vector<char> buf(16,0);
  std::snprintf (&buf[0],15,"%4.0f %s", t, u.c_str() );
  return std::string(&buf[0]);
}
} }//end fmr::perf namespace

//end FMR_API_PERF_HPP
#endif
