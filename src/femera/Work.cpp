#include "core.h"
#include "../fmr/form.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  Work::~Work () {
  }
  fmr::Exit_int Work::init_list (int* argc, char** argv)
  noexcept {fmr::Exit_int err =0;
    const auto n = this->task_list.size ();
    for (fmr::Local_int i=0; i<n; i++) {// Init task_list forward.
      const auto W =this->task_list[i].get();
      if (W != nullptr) {
#ifdef FMR_DEBUG
        printf ("Work: init list %s\n", W->name.c_str());
#endif
        W->time.add_idle_time_now ();
        const int Werr = W->init (argc, argv); // is noexcept
        err = (Werr == 0) ? err : Werr;
        W->time.add_busy_time_now ();
        const auto busy = fmr::form::si_unit_string (W->time.get_busy_s(),"s");
        const auto tot  = fmr::form::si_unit_string (W->time.get_work_s(),"s");
        if (W->data != nullptr) {
          const auto head = W->abrv+" init busy";
          const auto text = busy+" /"+tot;
          if (W->data->did_logs_init) {
            W->data->head_time (W->data->fmrlog, head, text);
          } else {
          if (this->proc != nullptr) {
            if (this->proc->is_main ()) {
              form::head_line (::stdout, 15, 80, head, text);
    } } } } } }
    if (err <=0) {this->did_work_init = true;}
    return err;
  }
  fmr::Exit_int Work::exit_list ()
  noexcept { fmr::Exit_int err =0;
    if (this->proc != nullptr && this->did_work_init == true) {
      this->is_work_main = this->proc->is_main ();
    }
    this->did_work_init = false;
    while (! this->task_list.empty ()) {
      auto W = this->task_list.back().get ();// Exit in reverse order.
      if (W != nullptr) {
#ifdef FMR_DEBUG
      printf ("Work: exit list %s\n", W->name.c_str());
#endif
      W->time.add_idle_time_now ();
      const fmr::Exit_int Werr = W->exit (err);// is noexcept
      err = (Werr == 0) ? err : Werr;
      W->time.add_busy_time_now ();
      const auto busy = fmr::form::si_unit_string (W->time.get_busy_s(),"s");
      const auto tot  = fmr::form::si_unit_string (W->time.get_work_s(),"s");
      const auto head = W->abrv+" exit busy";
      const auto text = busy+" /"+tot;
      if (W->data == nullptr) {
        if (this->is_work_main) {
          form::head_line (::stdout, 15, 80, head, text);
      } } else {
        W->data->head_line (W->data->fmrlog, head, text);
    } }
      this->task_list.pop_back ();
    }
    return err;
  }
  fmr::Exit_int Work::exit_tree ()
  noexcept { fmr::Exit_int err =0;
    if (this->proc != nullptr && this->did_work_init == true) {
      this->is_work_main = this->proc->is_main ();
    }
    this->did_work_init = false;
    Work::Task_path_t branch ={};
#ifdef FMR_DEBUG
    printf ("Work: exit tree %s [%lu]\n", this->name.c_str(), task_list.size());
#endif
    auto W = this;
    while (! W->task_list.empty ()) {// Go to the bottom of the hierarchy.
      branch.push_back (W->get_task_n () - 1);
#ifdef FMR_DEBUG
      printf ("Work: exit down (%u tasks) %s\n",W->get_task_n(),W->name.c_str());
#endif
      W = W->task_list.back ().get();
    }
    if ( W != nullptr) {
#ifdef FMR_DEBUG
      printf ("Work: exit branch 1 %s\n", W->name.c_str());
#endif
      W->time.add_idle_time_now ();
#if 0
      const fmr::Exit_int Werr = W->exit (err);// is noexcept
      err = (Werr == 0) ? err : Werr;
#else
      err= W->exit (err);// is noexcept
#endif
      W->time.add_busy_time_now ();
      const auto busy = fmr::form::si_unit_string (W->time.get_busy_s(),"s");
      const auto tot  = fmr::form::si_unit_string (W->time.get_work_s(),"s");
      const auto head = W->abrv+" exit busy";
      const auto text = busy+" /"+tot;
      if (W->data == nullptr) {
        if (this->is_work_main) {
          form::head_line (::stdout, 15, 80, head, text);
      } } else {
        W->data->head_line (W->data->fmrlog, head, text);
    } }
    if (!branch.empty ()) {
      branch.pop_back ();
      if (!branch.empty ()) {
        W = this->get_work_raw (branch);
        if (W !=nullptr) {
#ifdef FMR_DEBUG
          printf ("Work: exit pop %s\n", W->task_list.back()->name.c_str());
#endif
          if (!W->task_list.empty()) { W->task_list.pop_back(); }
#ifdef FMR_DEBUG
          printf ("Work: exit this %s\n", this->name.c_str());
#endif
          if (W!=this) {this->exit_tree(); }
    } } }
    return err;
  }
}//end femera:: namespace
#undef FMR_DEBUG





// FMR_SMART_PTR<Work> work = FMR_MAKE_SMART<Work>(some_work);
// FMR_SMART_PTR<Work> work(new Work(some_work));

#if 0
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
}
#endif
