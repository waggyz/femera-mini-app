#ifndef FEMERA_HAS_PROC_IPP
#define FEMERA_HAS_PROC_IPP

#include "../fmr/proc.hpp"

#include <string>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T> inline
  T* Proc<T>::derived (Proc* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  T* Proc<T>::derived (Work* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  Proc<T>* Proc<T>::this_cast (Work* ptr)
  noexcept {
    return static_cast<Proc<T>*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::init (int* argc, char** argv)
  noexcept {
    fmr::Exit_int err=0;
    try { Proc::derived (this)->task_init (argc, argv); }// Init this task,...
    catch (const Errs& e) { err = 1; e.print (); }
    catch (std::exception& e) { err = 2;
      femera::Errs::print (this->abrv+" task_init", e); }
    catch (...) { err = 3; }
    Work::init_list (argc, argv);// ...then init the sub-tasks.
    this->set_base_n ();
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::exit (fmr::Exit_int err)
  noexcept {
    this->data = nullptr;
    if (!this->task_list.empty()) {
      Work::exit_tree ();// Exit the task tree below this (is noexcept), ...
    }
    fmr::Exit_int task_err =0;
    try { Proc::derived (this)->task_exit (); }//...then exit this derived task.
    catch (const Errs& e) { task_err = 1; e.print (); }
    catch (std::exception& e) { task_err = 2;
      femera::Errs::print (this->abrv+" task_exit", e); }
    catch (...) { task_err = 3; }
    return (task_err > 0) ? task_err : err;
  }
  template <typename T> inline
  std::string Proc<T>::get_base_name ()
  noexcept {
    return "proc";
  }
#if 0
  template <typename T> inline
  FMR_SMART_PTR<T> Proc<T>::get_task_spt (const fmr::Local_int i) {
    return std::static_pointer_cast<T> (Work::get_work_spt (i));
  }
  template <typename T> inline
  FMR_SMART_PTR<T> Proc<T>::get_task_spt (const Work::Task_path_t path) {
    return std::static_pointer_cast<T> (Work::get_work_spt (path));
  }
#endif
  template <typename T> inline
  T* Proc<T>::get_task (const fmr::Local_int i) {
    return Proc::derived (Work::get_work_raw (i));
  }
  template <typename T> inline
  T* Proc<T>::get_task (const Work::Task_path_t path) {
    return Proc::derived (Work::get_work_raw (path));
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Proc<T>::new_task ()
  noexcept {
    return FMR_MAKE_SMART(T) (T());
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Proc<T>::new_task (const Work::Core_ptrs_t core)
  noexcept {
#if 0
    return std::move(FMR_MAKE_SMART<T> (T(core)));
#else
    return FMR_MAKE_SMART(T) (T(core));
#endif
  }
#if 0
  template <typename T> inline
  bool Proc<T>::is_main (bool ans=true) {// recursive version cannot inline
    ans = ans & (this->get_proc_ix () == this->main_ix);
    if (! this->task_list.empty ()) {
      ans = this->get_task (0)->is_main (ans);
    }
    return ans;
  }
#else
  template <typename T> inline
  bool Proc<T>::is_main () { bool ans=true;// series version
    Proc* P = this;
    ans = ans & (P->get_proc_ix () == P->main_ix);
    while (! P->task_list.empty ()) {
      P = static_cast<Proc*> (P->get_work_raw (0));//TODO only path 0? 
      ans = ans & (P->get_proc_ix () == P->main_ix);
    }
    return ans;
  }
#endif
#if 0
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_id () {fmr::Local_int id=0;
    Proc* P = this;
    id += P->base_id + P->base_n * P->get_proc_ix ();
    while (! P->task_list.empty ()) {
      P = static_cast<Proc*> (P->get_work_raw (0));
      id += P->base_id + P->base_n * P->get_proc_ix ();
#ifdef FMR_DEBUG
      printf ("%s id: %u += %u + %u * %u\n", P->abrv.c_str(), id,
        P->base_id, P->base_n, P->get_proc_ix ());
#endif
    }
    return id;
  }
#else
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_id (fmr::Local_int id) {
    const fmr::Local_int tid
      = this->base_id + this->base_n * Proc::derived (this)->get_proc_ix ();
    if (! this->task_list.empty ()) {
      id += this->get_task (0)->get_proc_id (tid);//TODO only path 0?
#ifdef FMR_DEBUG
      printf ("%s id: %u += %u + %u * %u\n", this->abrv.c_str(), id,
        this->base_id, this->base_n, Proc::derived (this)->get_proc_ix ());
#endif
    }
    return id;
  }
#endif
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_ix () {
#ifdef FMR_DEBUG
    printf ("Proc<%s>>::get_proc_ix: %u\n", this->abrv.c_str(),
      Proc::derived (this)->task_proc_ix ());
#endif
    return Proc::derived (this)->task_proc_ix ();
    // proc->task_proc_ix() always calls Main::task_proc_ix ().
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_n ()
  noexcept {
#if 0
    this->proc_n =0;
    for (const auto W : this->task_list) {
      const auto P = static_cast<Proc<T>*> (W.get());
      this->proc_n += P->get_proc_n ();
    }
#endif
    return this->proc_n;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::all_proc_n () {fmr::Local_int n=1;
    auto P = this;
    while (! P->task_list.empty ()) {
      n *= P->get_proc_n ();
#ifdef FMR_DEBUG
      printf ("%s: * %u = %u\n", P->name.c_str(), P->get_proc_n(), n);
#endif
      P = this_cast (P->get_work_raw (0));
    }
    return n;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::set_base_n () {
    auto P = this;
    while (! P->task_list.empty ()) {
      P->base_n = P->all_proc_n () / P->proc_n;
      P = this_cast (P->get_work_raw (0));
    }
    return this->base_n;
  }
  template <typename T> inline
  proc::Team_t Proc<T>::get_team_id ()
  noexcept {
    return this->team_id;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_team_n ()
  noexcept {
    return fmr::Local_int (this->task_list.size());
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_PROC_IPP
#endif