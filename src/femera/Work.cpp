#include "core.h"
#include "../fmr/form.hpp"

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
        if (Werr>0) { del_list.push (ix); }
        err = (Werr >= 0) ? err : Werr;
        W->time.add_busy_time_now ();
        if (W->data != nullptr) {
          const auto busy = fmr::form::si_time_string (W->time.get_busy_s());
          const auto tot  = fmr::form::si_time_string (W->time.get_work_s());
          if (Werr > 0) {// Remove task if failed to initialize.
            const auto head = femera::form::text_line (250, "%4s %4s init",
              W->get_base_name().c_str(), W->abrv.c_str());
            const auto text = busy+" /"+tot+" failed: "+W->name
              +" returned "+std::to_string(Werr);
            if (W->data->did_logs_init) {
              W->data->head_line (W->data->fmrlog, head, text);
            } else {
            if (W->proc != nullptr) {
              if (W->proc->is_main ()) {
                form::head_line (::stdout, 14, 80, head, text);
            } } }
            W->exit (-1);
          } else {
            const auto head = femera::form::text_line (250, "%4s %4s init",
              W->get_base_name().c_str(), W->abrv.c_str());
            const auto text = busy+" /"+tot+" "+W->name
              +((W->version=="") ? "":" "+W->version);
            if (W->data->did_logs_init) {
              W->data->head_line (W->data->fmrlog, head, text);
            } else {
            if (W->proc != nullptr) {
              if (W->proc->is_main ()) {
                form::head_line (::stdout, 14, 80, head, text);
    } } } } } } }
    while (! del_list.empty ()) {
      const auto head = femera::form::text_line (250, "%4s %4s init",
        this->get_base_name().c_str(), this->abrv.c_str());
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
      err = (Werr == 0) ? err : Werr;
      W->time.add_busy_time_now ();
      const auto busy = fmr::form::si_time_string (W->time.get_busy_s());
      const auto tot  = fmr::form::si_time_string (W->time.get_work_s());
      const auto head = femera::form::text_line (250, "%4s %4s exit",
        W->get_base_name().c_str(), W->abrv.c_str());
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
      const auto busy = fmr::form::si_time_string (W->time.get_busy_s());
      const auto tot  = fmr::form::si_time_string (W->time.get_work_s());
      const auto head = femera::form::text_line (250, "%4s %4s exit",
        W->get_base_name().c_str(), W->abrv.c_str());
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
          if (!W->task_list.empty()) { W->task_list.pop_back(); }
#ifdef FMR_DEBUG
          printf ("Work: exit this %s\n", this->name.c_str());
#endif
          if (W!=this) {this->exit_tree(); }
    } } }
    return err;
  }
  fmr::Local_int Work::add_task (Work_spt W)// inlining fails growth limit
  noexcept {
    this->task_list.push_back (std::move(W));
    return fmr::Local_int (this->task_list.size () - 1);
  }
}//end femera:: namespace
#undef FMR_DEBUG
