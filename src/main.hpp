#ifndef FMR_API_MAIN_HPP
#define FMR_API_MAIN_HPP
#include "core.h"

namespace fmr {// namespace main or core? TODO
  extern int init (int* argc, char** argv);
  extern int exit (int err);
}//end fmr:: namespace

namespace fmr { namespace sims {
  extern int add (const std::string model_name);
  extern int add (const std::deque<std::string> model_names);
  extern int clear ();
  extern int run ();
  extern int run_file (const std::string file_name);// Implemented in Data.cpp.
} }//end fmr::sims:: namespace

namespace fmr { namespace detail {
  extern Femera::Main* main;//TODO singleton?
} }//end fmr::detail namespace

// hmm... Order is important...
//#include "proc.hpp"
//#include "flog.hpp"
//#include "data.hpp"
//#include "sims.hpp"
//#include "phys.hpp"
#if 0
//Empty files may be included here so they show in the dependency tree.
//#include "proc.hpp"
//#include "flog.hpp"
namespace fmr{ namespace proc{
extern inline bool is_master ();
extern inline int  barrier   ();
} }

// Thin wrappers are inline and defined here.
bool fmr::proc:: is_master (){return fmr::detail::main->proc-> is_master (); }
int  fmr::proc:: barrier   (){return fmr::detail::main->proc-> barrier   (); }

namespace fmr{ namespace flog{

static inline int get_verbosity ();
static inline int get_detail    ();
static inline int get_timing    ();

template<typename ...Args>// only prints from master
int label_printf (std::string label, std::string format, Args ...args );

// Not much point inlining the rest?
static int print_heading (std::string);

static std::string print_label_units_meter (const std::string label,
  const std::string unit, const fmr::perf::Meter timer );

// Implementation =============================================================
static inline int get_verbosity (){
  return fmr::detail::main->log-> verbosity;
}
static inline int get_detail (){
  return fmr::detail::main->log-> detail;
}
static inline int get_timing (){
  return fmr::detail::main->log-> timing;
}

//TODO Move the rest to Flog.cpp ?
static int print_heading (std::string text ){
  return fmr::detail::main->log-> print_heading (text.c_str());
}
template<typename ...Args> int label_printf (
   std::string label, std::string format, Args ...args ){
    return fmr::detail::main->log-> label_printf (
      label.c_str(), format.c_str(), args...);
}
#if 0
template<typename ...Args> int label_printf_err (
   std::string label, std::string format, Args ...args ){
    return fmr::detail::main->log-> label_printf_err (
      label.c_str(), format.c_str(), args...);
}
#endif
#if 1
static std::string print_label_units_meter (std::string label,
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
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include main.hpp")
//end FMR_API_MAIN_HPP
#endif
