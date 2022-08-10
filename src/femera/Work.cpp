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
  Work::Work (Work &&) = default;// movable, here to avoid -Winline warns
  Work::~Work () {
  }
  fmr::Local_int Work::add_task (Work_spt W)
  noexcept {
    this->task_list.push_back (std::move (W));
    return fmr::Local_int (this->task_list.size () - 1);// index of task added
  }
  Work* Work::get_work (const fmr::Local_int ix)
  noexcept {
    return (ix < this->task_list.size()) ? this->task_list [ix].get() : nullptr;
  }
  Work* Work::get_work (const Work_type t, const fmr::Local_int ix)
  noexcept {
    fmr::Local_int i = 0;
    auto W = this;
#if 1
    if (W->task_type == t) {
      //NOTE Task 0 is the parent, with 1-indexed children of the same type.
      if (i == ix) { return W; }
      ++i;
    }
#endif
    while (! W->task_list.empty ()) {
      const fmr::Local_int n = W->get_task_n ();
      for (fmr::Local_int Wix = 0; Wix < n; ++Wix) {
        auto Wchild = W->task_list [Wix].get();
        if (Wchild->task_type == t) {
          if (i == ix) { return Wchild; }
          ++i;
      } }
#if 0
      for (fmr::Local_int Wix = 0; Wix < n; ++Wix) {
        W = W->task_list [Wix].get();
      }// this is: W = W->get_work (n-1);
#else
      W = W->task_list [0].get();//NOTE only descends 0-indexed task branch
      if (W == nullptr) { return nullptr; }
#endif
    }
    return nullptr;
  }
  fmr::Local_int Work::log_init_list ()
  noexcept { fmr::Local_int did_init_count = 0;
    const auto n = this->get_task_n ();
    if ((n > 0) && (this->proc != nullptr)) {
      if (this->proc->did_init () && this->proc->is_main ()) {
        auto did_str = std::string ();
        auto not_str  = std::string ();;
        fmr::Local_int did_n = 0, not_n=0;
        for (fmr::Local_int i = 0; i < n; ++i) {
          const auto W = this->get_work (i);
          const auto item = W->get_abrv ();
          if (W->did_init ()) {
            ++did_init_count;
            if (fmr::form::ends_with (did_str, item)) {
              ++did_n;// repeated item
            } else {  // different item
              if (did_n > 1) { did_str += "("+std::to_string (did_n)+")"; }
              did_n= 1;
              did_str += (did_str.size () == 0) ? "":" ";
              did_str += item;
          } } else {// did not init
            if (fmr::form::ends_with (not_str, item)) {
              ++not_n;// repeated item
            } else {  // different item
              if (not_n > 1) { not_str += "("+std::to_string (not_n)+")"; }
              not_n = 1;
              not_str += ( not_str.size () == 0) ? "":" ";
              not_str += item;
        } } }
        if (did_str.size() > 0) {
          if (did_n > 1) {did_str += "("+std::to_string (did_n)+")"; }
          printf ("%4s %4s %4s %4u ok /%4u %s: %s\n",
            get_base_abrv ().c_str(), get_abrv ().c_str(),"init",
              did_init_count, n, get_base_abrv ().c_str(), did_str.c_str());
        }
        if (not_str.size() > 0) {
          if (not_n > 1) { not_str += "("+std::to_string (not_n)+")"; }
          printf ("%4s %4s %4s %4u :( /%4u %s: %s\n",
            get_base_abrv ().c_str(), get_abrv ().c_str(), "nope",
              n - did_init_count, n, get_base_abrv ().c_str(), not_str.c_str());
    } } }
    return did_init_count;
  }
  fmr::Exit_int Work::init_list (int* argc, char** argv)
  noexcept { fmr::Exit_int err =0;
    std::stack<fmr::Local_int> del_list = {};
    const auto n = this->task_list.size ();
    for (fmr::Local_int ix = 0; ix < n; ++ix) {// Init task_list forward.
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
          if (! W->task_list.empty ()) {// sum children busy time
            const auto nC = fmr::Local_int (W->task_list.size ());
            for (fmr::Local_int i = 0; i < nC; ++i) {
              const auto C = W->get_work (i);
              if (C != nullptr) {
                child_busy_ns += C->time.get_busy_ns ();
          } } }
          const auto busy_s = fmr::perf::Float (1e-9)
            * (fmr::perf::Float (W->time.get_busy_ns () - child_busy_ns));
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
              const auto label = form::text_line (250, "%4s %4s init",
                W->get_base_abrv ().c_str(), W->abrv.c_str());
              form::name_line (::stdout, 14, 80, label, text);
        } } } }
        if (Werr > 0 || (W->did_init () == false)) {
          del_list.push (ix);// Queue task for removal if init failed.
    } } }
    if (n > 0) { this->log_init_list (); }
    while (! del_list.empty ()) {// Remove failed tasks.
      const auto label = form::text_line (250, "%4s %4s init",
        this->get_base_abrv ().c_str(), this->get_abrv().c_str());
      form::name_line (::stdout, 14, 80, label, "removing %s...",
        this->get_work (del_list.top ())->get_name ().c_str());
      this->del_task (del_list.top ());
      del_list.pop ();
    }
    if (err <= 0) { this->set_init (true); }//NOTE should be redundant to init
    return err;
  }
  fmr::Exit_int Work::exit_list ()
  noexcept { fmr::Exit_int err = 0;
    if ((this->proc != nullptr) && (this->did_init ())) {
FMR_WARN_INLINE_OFF
      this->is_work_main_tf = this->proc->is_main ();
FMR_WARN_INLINE_ON
    }
    this->set_init (false);//NOTE should be redundant to derived::exit (..)
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
        auto child_busy_ns = fmr::perf::Elapsed (0);
        if (! W->task_list.empty ()) {// calculate children busy time
          const auto n = fmr::Local_int (W->task_list.size ());
          for (fmr::Local_int i = 0; i < n; ++i) {
            const auto C = W->get_work (i);
            if (C != nullptr) {
              child_busy_ns += C->time.get_busy_ns ();
        } } }
        const auto busy_s = fmr::perf::Float (1e-9)
          * (fmr::perf::Float (W->time.get_busy_ns () - child_busy_ns));
#endif
        this->exit_info (W, busy_s);
      }
      this->task_list.pop_back ();
    }
    return err;
  }
  fmr::Exit_int Work::exit_tree ()
  noexcept { fmr::Exit_int err = 0;
    if (this->proc != nullptr && this->did_init ()) {
FMR_WARN_INLINE_OFF
      this->is_work_main_tf = this->proc->is_main ();
FMR_WARN_INLINE_ON
    }
    this->set_init (false);//NOTE should be redundant to derived::exit (..)
    if (! this->task_list.empty ()) {
      Work::Task_path_t branch ={};
#ifdef FMR_DEBUG
      printf ("Work: exit tree %s [%lu]\n",
        this->name.c_str(), this->task_list.size());
#endif
      auto W = this;
      while (! W->task_list.empty ()) {// Go to the bottom of the hierarchy.
        branch.push_back (W->get_task_n () - 1);
#ifdef FMR_DEBUG
        printf("Work: exit down (%u tasks) %s\n",
          W->get_task_n(), W->name.c_str());
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
    //TODO change to task_time_info (..) and use in init_* and exit_* methods
    fmr::Exit_int err=0;
    const auto busy = fmr::form::si_time (busy_s);
    auto read = std::string ();
    auto save = std::string ();
    auto flop = std::string ();
    if (W->time.get_read_n () > fmr::Perf_int (0)) {
      read = form::perf_line (W->time.get_read (), busy_s);
    }
    if (W->time.get_save_n () > fmr::Perf_int (0)) {
      save = form::perf_line (W->time.get_save (), busy_s);
    }
    if (W->time.get_flop_n () > fmr::Perf_int (0)
      && W->time.get_byte_n () > fmr::Perf_int (0)) {
      flop="**** flop";
      //flop = form::flop_line//TODO
      //(this->time.get_flop (), this->time.get_byte (), busy_s);
    }
    const auto tot  = fmr::form::si_time (W->time.get_work_s ());
    const auto text = busy+" /"+tot+" "+W->name
      + ((W->version=="") ? "":" "+W->version);
FMR_WARN_INLINE_OFF
    if (W->data == nullptr) {
      if (this->is_work_main_tf) {
        const auto label = form::text_line (80, "%4s %4s ",
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
      } }
    else {
      W->data->send (fmr::log, W->get_base_abrv(), W->get_abrv(), "exit", text);
      if (read.size () > 0) { W->data->send
        ("fmr:perf", W->get_base_abrv(), W->get_abrv(),"inp", read);
      }
      if (save.size () > 0) { W->data->send
        ("fmr:perf", W->get_base_abrv(), W->get_abrv(),"out", save);
      }
      if (flop.size () > 0) { W->data->send
        ("fmr:perf", W->get_base_abrv(), W->get_abrv(),"flop",flop);
      }
    }
FMR_WARN_INLINE_ON
    return err;
  }
}//end  namespace
#undef FMR_DEBUG
