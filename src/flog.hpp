#ifndef FMR_API_FLOG_HPP
#define FMR_API_FLOG_HPP
/** */
#include "proc.hpp"
#if 1
namespace fmr{ namespace flog{

static inline int get_verbosity ();
static inline int get_detail    ();
static inline int get_timing    ();

template<typename ...Args>// only prints from master
int label_printf (std::string label, std::string format, Args ...args );

//TODO Not much point inlining the rest?
//static inline std::string format_time_units (fmr::perf::Perf_int ns);
static inline int print_heading (std::string);
#if 0
static inline std::string print_label_units_meter (const std::string label,
  const std::string unit, const fmr::perf::Meter timer );
#endif
// Implementation =============================================================
static inline int get_verbosity (){
  return fmr::detail::main->proc->log-> verbosity;
}
static inline int get_detail (){
  return fmr::detail::main->proc->log-> detail;
}
static inline int get_timing (){
  return fmr::detail::main->proc->log-> timing;
}
template<typename ...Args> std::string format_label_line (
   std::string label, std::string format, Args ...args ){
    return fmr::detail::main->proc->log->format_label_line (
      label.c_str(), format.c_str(), args...);
}
template<typename ...Args> int label_printf (
   std::string label, std::string format, Args ...args ){
    return fmr::detail::main->proc->log->label_printf (
      label.c_str(), format.c_str(), args...);
}
#if 0
template<typename ...Args> int label_printf_err (
   std::string label, std::string format, Args ...args ){
    return fmr::detail::main->proc->log-> label_printf_err (
      label.c_str(), format.c_str(), args...);
}
#endif

//TODO Move the rest to Flog.cpp ?
static inline int print_heading (std::string text ){
  fmr::detail::main->proc->log-> print_heading (text.c_str());
  return 0;
}
#if 0
static inline std::string format_time_units (fmr::perf::Perf_int ns){
  return fmr::detail::main->proc->log-> format_time_units (ns);
}
#endif
#if 0
static inline std::string print_label_units_meter (std::string label,
  const std::string unit, const fmr::perf::Meter timer ){
  if( fmr::flog:: get_verbosity ()> 1 ){
    label+=" time";
    fmr::flog:: label_printf( label,"%.3f s busy, %.3f s idle for %li %s\n",
      double( fmr::perf::timer_busy_ns(timer) )* 1e-9,
      double( fmr::perf::timer_idle_ns(timer) )* 1e-9, timer.count, unit.c_str() );
    double work_per_sec = fmr::perf::overall_speed ( timer );
    double sec_per_work = 1.0 / work_per_sec;
    double t=sec_per_work;
    std::string f="";
    if     ( sec_per_work < 2e-6 ){f="%.0f ns/"+unit+", %.1f "+unit+"/s\n";
      t*=1e9; }
    else if( sec_per_work < 0.002){f="%.0f \u00b5s/"+unit+", %.1f "+unit+"/s\n";
      t*=1e6; }
    else if( sec_per_work <   2.0){f="%.0f ms/"+unit+", %.1f "+unit+"/s\n";
      t*=1e3; }
    else if( sec_per_work < 120.0){f="%.0f s /"+unit+"\n"; }
    else if( sec_per_work <7200.0){f="%.0f mn/"+unit+"\n"; t/=  60.0; }
    else                          {f="%.0f hr/"+unit+"\n"; t/=3600.0; }
    if( sec_per_work < 1.0 ){
      fmr::flog:: label_printf ("Overall Sims", f, t, work_per_sec);
    }else{
      fmr::flog:: label_printf ("Overall Sims", f, t );
  } }
  return std::string("");//TODO
}
#endif
} }//end fmr::flog namespace
#endif
#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include flog.hpp")
//end FMR_API_FLOG_HPP
#endif
