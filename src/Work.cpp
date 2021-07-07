#include "core.h"

#include <string>
#include <cstdio>     // std::printf
//#include <memory>     // std::unique_ptr, std::make_unique

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace Femera{
Work::~Work () {}
int Work::prep () {return 0;}
int Work::chck () {return 0;}
int Work::init (int* argc, char** argv) {int err=0;
#if 0
  err = this->prep();
  if(err){return err; }
  return this->task.init_stack( argc,argv );
  fmr::perf::timer_pause (& this->time );
#else
  fmr::perf::timer_start (&this->time);
  err= this->prep ();
  if (err) {fmr::perf::timer_pause (&this->time); return err;}
  err= this->task.init_stack (argc,argv);
  if (err) {fmr::perf::timer_pause (&this->time); return err;}
  Work::print_summary ();
  this->print_task_time ("init");
  fmr::perf::timer_pause (&this->time);
  return err;
#endif
}
int Work::exit (int err) {//TODO Why getting called a lot (after task empty)
  // when exiting Proc?
#ifdef FMR_DEBUG
  std::printf ("**** Work::exit (%i) %s...\n", err, this->task_name.c_str());
#endif
  this->print_task_time ("done");
  fmr::perf::timer_resume (&this->time);
  if (this->task.count () > 0) {
    err= this->task.exit_stack (err);
  }
  fmr::perf::timer_pause (&this->time);
  return err;
}
std::string Work::print_summary () {
  std::string label, info("");
  const int n = this->task.count();
  if (n == 0) {
    label = this->task_name+" mods";
    info  = "no modules loaded";
  }
  for (int i=0; i<n; i++) {
    if (i==0) {
      label = this->task.get<Work>(i)->task_name+" mods";
    }
    else {
      info += this->task.get<Work>(i)->task_name;
    }
    if (this->task.get<Work>(i)->version.size()) {
      info += std::string(" ");
      info += this->task.get<Work>(i)->version;
    }
    if ((i>0) && (i+1 < n)) {
      info += std::string(", ");
  } }
  return this->proc->log->print_label_line (label, info);
}
std::string Work::print_details () {
  return this->print_summary ();
}
int Work::print_task_time (const std::string name_suffix) {
  const auto log = this->proc->log;
  if (log->timing >= this->verblevel && log->verbosity >= this->verblevel) {
//    bool did_print_this = false;//TODO
    if (log->detail >= this->verblevel) {// Print details: task times.
      const int n = this->task.count ();
      if (n>0) {for (int i=n-1; i>=0; i--) {
        Work* W = this->task.get<Work>(i); if (W) {
//          if (i>0) {fmr::perf::timer_resume (&W->time);}//TODO Use below?
          if (W == this) {//did_print_this = true;}
          } else {fmr::perf::timer_resume (&W->time);}
          fmr::perf::timer_pause (&W->time);
          if (log->timing >= W->verblevel) {
            const auto  busy = double (timer_busy_ns (W->time));
            const auto total = double (timer_total_ns(W->time));
            if (total > 0.0) {
              if (W->verblevel < 8 || (busy > 0.002*total)) {//TODO magic nmbrs
                const auto label = W->task_name+" "+name_suffix;
                W->proc->log->print_label_meter (label, W->meter_unit, W->time);
    } } } } } } } else {//FIXME not working as intended for no detail Proc exit.
//    if (!did_print_this) {// Print a summary of this timing.
    fmr::perf::timer_resume (&this->time);
    const auto label = this->task_name+" "+name_suffix;
    log->print_label_meter (label, this->meter_unit, this->time);
    fmr::perf::timer_pause (&this->time);
  } }
  return 0;
}
}// end Femera namespace
#undef FMR_DEBUG
