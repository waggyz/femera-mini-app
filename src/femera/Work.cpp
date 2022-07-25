#include "core.h"

#include <stack>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  Work::Work () noexcept =default;
  Work::Work (const Work::Core_ptrs_t core) noexcept {
    std::tie (this->proc, this->data, this->test) = core;
  }
#if 0
  Work::Work (Work const&) = default;// copyable, here to avoid -Winline warns
#endif
  Work::Work (Work &&) = default;// movable, here to avoid -Winline warns
  Work::~Work () {
  }
  fmr::Local_int Work::add_task (Work_spt W)
  noexcept {
    this->task_list.push_back (std::move(W));
    return fmr::Local_int (this->task_list.size () - 1);// index of task added
  }
  Work* Work::get_work (const fmr::Local_int ix)
  noexcept {
    return (ix < this->task_list.size()) ? this->task_list [ix].get() : nullptr;
  }
  Work* Work::get_work (const Work_type t, const fmr::Local_int ix)
  noexcept {
    fmr::Local_int i=0;
    auto W = this;
#if 1
    if (W->task_type == t) {
      //TODO Is this the desired behavior of nested drivers of the same type?
      //     Task 0 is the parent, with 1-indexed children of the same type.
      if (i == ix) { return W; }
      ++i;
    }
#endif
    while (! W->task_list.empty ()) {
      const fmr::Local_int n = W->get_task_n ();
      for (fmr::Local_int Wix=0; Wix < n; ++Wix) {
        if (W->task_list [Wix].get()->task_type == t) {
          if (i == ix) { return W->task_list [Wix].get(); }
          ++i;
      } }
      for (fmr::Local_int Wix=0; Wix < n; ++Wix) {
        W = W->task_list [Wix].get();
      }
    }
    return nullptr;
  }
  fmr::Exit_int Work::init_list (int* argc, char** argv)
  noexcept {fmr::Exit_int err =0;
    const auto n = this->task_list.size ();
    std::stack<fmr::Local_int> del_list = {};
    for (fmr::Local_int ix=0; ix<n; ++ix) {// Init task_list forward.
      const auto W = this->task_list [ix].get();
      if (W != nullptr) {
#ifdef FMR_DEBUG
        printf ("Work: init list %s\n", W->get_name ().c_str());
#endif
        W->time.add_idle_time_now ();
        const int Werr = W->init (argc, argv); // is noexcept
        W->time.add_busy_time_now ();
        err = (Werr >= 0) ? err : Werr;
        if (W->data != nullptr) {
#if 0
          const auto busy_s = W->time.get_busy_s()
#else
          auto child_busy_ns = fmr::perf::Elapsed (0);
          if (! W->task_list.empty ()// sum children busy time
            && W->task_type != task_cast (Task_type::Main)) {
            const auto nC = fmr::Local_int (W->task_list.size ());
            for (fmr::Local_int i=0; i<nC; ++i) {
              const auto C = W->get_work (i);
              if (C!= nullptr) {
                child_busy_ns += C->time.get_busy_ns ();
          } } }
          const auto busy_s = fmr::perf::Float (1e-9)
            * ((W->time.get_busy_ns () > child_busy_ns)
              ? fmr::perf::Float (W->time.get_busy_ns () - child_busy_ns)
              : fmr::perf::Float (0.0));
#endif
          const auto busy = fmr::form::si_time (busy_s);
          const auto tot  = fmr::form::si_time (W->time.get_work_s ());
          std::string text = "";
          if (Werr > 0) {
            text = busy+" /"+tot+" failed: "+W->get_name ()
              +" returned "+std::to_string (Werr);
          } else {
            text = busy+" /"+tot+" "+W->get_name ()
              +((W->version=="") ? "":" "+W->version);
          }
          if (W->data->did_logs_init ()) {
            W->data->send (fmr::log,
              W->get_base_abrv (), W->get_abrv (), "init", text);
          } else {
          if (W->proc != nullptr) {
            if (W->proc->is_main ()) {
              const auto label = femera::form::text_line (250, "%4s %4s init",
                W->get_base_abrv ().c_str(), W->abrv.c_str());
              femera::form::name_line (::stdout, 14, 80, label, text);
          } } }
        }
        if (Werr > 0) {
          del_list.push (ix);// Queue task for removal if init failed, and...
          W->exit (-1);      // ...exit it with a warning (not error) code.
    } } }
    while (! del_list.empty ()) {// Remove failed tasks.
      const auto label = femera::form::text_line (250, "%4s %4s init",
        this->get_base_abrv ().c_str(), this->get_abrv().c_str());
      femera::form::name_line (::stdout, 14, 80, label, "removing %s...",
        this->get_work (del_list.top ())->get_name ().c_str());
      this->del_task (del_list.top ());
      del_list.pop ();
    }
    if (err <=0) {this->did_work_init = true;}
    return err;
  }
  fmr::Exit_int Work::exit_list ()
  noexcept { fmr::Exit_int err =0;
    if (this->proc != nullptr && this->did_work_init == true) {
FMR_WARN_INLINE_OFF
      this->is_work_main = this->proc->is_main ();
FMR_WARN_INLINE_ON
    }
    this->did_work_init = false;
    while (! this->task_list.empty ()) {
      auto W = this->task_list.back ().get ();// Exit in reverse order.
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
        if (! W->task_list.empty ()) {// calculate children busy time
          const auto n = fmr::Local_int (W->task_list.size ());
          for (fmr::Local_int i=0; i<n; ++i) {
            const auto C = W->get_work (i);
            if (C!= nullptr) {
              child_busy_ns += C->time.get_busy_ns ();
        } } }
        const auto busy_s = fmr::perf::Float (1e-9)
          * ((W->time.get_busy_ns () > child_busy_ns)
            ? fmr::perf::Float (W->time.get_busy_ns () - child_busy_ns)
            : fmr::perf::Float (0.0));
#endif
        this->exit_info (W, busy_s);
      }
      this->task_list.pop_back ();
    }
    return err;
  }
  fmr::Exit_int Work::exit_tree ()
  noexcept { fmr::Exit_int err =0;
    if (this->proc != nullptr && this->did_work_init == true) {
FMR_WARN_INLINE_OFF
      this->is_work_main = this->proc->is_main ();
    }
FMR_WARN_INLINE_ON
    this->did_work_init = false;
    if (! this->task_list.empty ()) {
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
        this->exit_info (W, busy_s);
      }
      if (! branch.empty ()) {
        branch.pop_back ();
        if (! branch.empty ()) {
          W = this->get_work (branch);
          if (W != nullptr) {
#ifdef FMR_DEBUG
            printf ("Work: exit pop  %s\n", W->task_list.back ()->name.c_str());
#endif
            if (! W->task_list.empty()) {W->task_list.pop_back ();}
#ifdef FMR_DEBUG
            printf ("Work: exit this %s\n", this->name.c_str());
#endif
            if (W != this) {this->exit_tree ();}
    } } } }
    return err;
  }
  fmr::Exit_int Work::exit_info (Work* W, const fmr::perf::Float busy_s) {
    fmr::Exit_int err=0;
    const auto busy = fmr::form::si_time (busy_s);
    auto read = std::string ();
    auto save = std::string ();
    auto flop = std::string ();
    if (W->time.get_read_n () > fmr::Perf_int (0)) {
//      read="**** inp";
      read = femera::form::perf_line (W->time.get_read (), busy_s);
    }
    if (W->time.get_save_n () > fmr::Perf_int (0)) {
//      save="**** out";
      save = femera::form::perf_line (W->time.get_save (), busy_s);
    }
    if (W->time.get_flop_n () > fmr::Perf_int (0)
      && W->time.get_byte_n () > fmr::Perf_int (0)) {
      flop="**** flop";
      //flop = femera::form::flop_line
      //(this->time.get_flop (), this->time.get_byte (), busy_s);
    }
    const auto tot  = fmr::form::si_time (W->time.get_work_s ());
    const auto text = busy+" /"+tot+" "+W->name
      + ((W->version=="") ? "":" "+W->version);
FMR_WARN_INLINE_OFF
    if (W->data == nullptr) {
      if (this->is_work_main) {
      const auto label = femera::form::text_line (250, "%4s %4s ",
        W->get_base_abrv().c_str(), W->get_abrv().c_str());
      form::name_line (::stdout, 14, 80, label+"exit", text);
      if (read.size () > 0) {
        form::name_line (::stdout, 14, 80, label+" inp", read);
      }
      if (save.size () > 0) {
        form::name_line (::stdout, 14, 80, label+" out", save);
      }
      if (flop.size () > 0) {
        form::name_line (::stdout, 14, 80, label+"flop", flop);
      }
    } } else {
      W->data->send (fmr::log, W->get_base_abrv(), W->get_abrv(), "exit", text);
      if (read.size () > 0) {
        W->data->send (fmr::log, W->get_base_abrv(), W->get_abrv(),"inp", read);
      }
      if (save.size () > 0) {
        W->data->send (fmr::log, W->get_base_abrv(), W->get_abrv(),"out", save);
      }
      if (flop.size () > 0) {
        W->data->send (fmr::log, W->get_base_abrv(), W->get_abrv(),"flop",flop);
      }
    }
FMR_WARN_INLINE_ON
    return err;
  }
}//end femera:: namespace
#undef FMR_DEBUG
