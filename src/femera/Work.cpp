#include "core.h"

#include <stack>

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
    std::stack<fmr::Local_int> del_list = {};
    for (fmr::Local_int ix=0; ix<n; ix++) {// Init task_list forward.
      const auto W =this->task_list[ix].get();
      if (W != nullptr) {
#ifdef FMR_DEBUG
        printf ("Work: init list %s\n", W->name.c_str());
#endif
        W->time.add_idle_time_now ();
        const int Werr = W->init (argc, argv); // is noexcept
        W->time.add_busy_time_now ();
        err = (Werr >= 0) ? err : Werr;
        if (W->data != nullptr) {
#if 1
          auto child_busy_ns = fmr::perf::Elapsed(0);
          if (! W->task_list.empty ()) {// calculate children busy time
            const auto nC = fmr::Local_int (W->task_list.size ());
            for (fmr::Local_int i=0; i<nC; i++) {
              const auto C = W->get_work_raw (i);
              if (C!= nullptr) {
                child_busy_ns += C->time.get_busy_ns ();
          } } }
          const auto busy_s = fmr::perf::Float(1e-9)
            * fmr::perf::Float((W->time.get_busy_ns() - child_busy_ns));
#else
          const auto busy_s = W->time.get_busy_s()
#endif
          const auto busy = fmr::form::si_time (busy_s);
          const auto tot  = fmr::form::si_time (W->time.get_work_s());
          std::string text = "";
          if (Werr > 0) {
            text = busy+" /"+tot+" failed: "+W->name
              +" returned "+std::to_string(Werr);
          } else {
            text = busy+" /"+tot+" "+W->name
              +((W->version=="") ? "":" "+W->version);
          }
          const auto head = femera::form::text_line (250, "%4s %4s init",
            W->get_base_name ().c_str(), W->abrv.c_str());
          if (W->data->did_logs_init) {
            W->data->head_line (W->data->fmrlog, head, text);
          } else {
          if (W->proc != nullptr) {
            if (W->proc->is_main ()) {
              form::head_line (::stdout, 14, 80, head, text);
        } } } }
        if (Werr > 0) {
          del_list.push (ix);// Queue task for removal if init failed, and...
          W->exit (-1);      // ...exit with a warning (not error) code.
    } } }
    while (! del_list.empty ()) {// Remove failed tasks.
      const auto head = femera::form::text_line (250, "%4s %4s init",
        this->get_base_name ().c_str(), this->abrv.c_str());
      form::head_line (::stdout, 14, 80, head, "removing %s...",
        this->get_work_raw (del_list.top ())->name.c_str());
      this->del_task (del_list.top ());
      del_list.pop ();
    }
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
        W->time.add_busy_time_now ();
        err = (Werr == 0) ? err : Werr;
#if 0
        const auto busy_s = W->time.get_busy_s ()
#else
        auto child_busy_ns = fmr::perf::Elapsed(0);
        if (! W->task_list.empty()) {// calculate children busy time
          const auto n = fmr::Local_int (W->task_list.size ());
          for (fmr::Local_int i=0; i<n; i++) {
            const auto C = W->get_work_raw (i);
            if (C!= nullptr) {
              child_busy_ns += C->time.get_busy_ns ();
        } } }
        const auto busy_s = fmr::perf::Float (1e-9)
          * fmr::perf::Float ((W->time.get_busy_ns () - child_busy_ns));
#endif
        const auto busy = fmr::form::si_time (busy_s);
        const auto tot  = fmr::form::si_time (W->time.get_work_s());
        const auto head = femera::form::text_line (250, "%4s %4s exit",
          W->get_base_name ().c_str(), W->abrv.c_str());
        const auto text = busy+" /"+tot+" "+W->name
          +((W->version=="") ? "":" "+W->version);
        if (W->data == nullptr) {
          if (this->is_work_main) {
            form::head_line (::stdout, 14, 80, head, text);
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
      printf("Work: exit down (%u tasks) %s\n",W->get_task_n(),W->name.c_str());
#endif
      W = W->task_list.back ().get();
    }
    if (W != nullptr) {
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
      const auto busy_s = W->time.add_busy_time_now ();
      const auto busy = fmr::form::si_time (busy_s);
      const auto tot  = fmr::form::si_time (W->time.get_work_s ());
      const auto head = femera::form::text_line (250, "%4s %4s exit",
        W->get_base_name ().c_str(), W->abrv.c_str());
      const auto text = busy+" /"+tot+" "+W->name
        +((W->version=="") ? "":" "+W->version);
      if (W->data == nullptr) {
        if (this->is_work_main) {
          form::head_line (::stdout, 14, 80, head, text);
        } } else {
        W->data->head_line (W->data->fmrlog, head, text);
    } }
    if (! branch.empty ()) {
      branch.pop_back ();
      if (! branch.empty ()) {
        W = this->get_work_raw (branch);
        if (W != nullptr) {
#ifdef FMR_DEBUG
          printf ("Work: exit pop %s\n", W->task_list.back()->name.c_str());
#endif
          if (! W->task_list.empty()) {W->task_list.pop_back();}
#ifdef FMR_DEBUG
          printf ("Work: exit this %s\n", this->name.c_str());
#endif
          if (W != this) {this->exit_tree();}
    } } }
    return err;
  }
}//end femera:: namespace
#undef FMR_DEBUG
