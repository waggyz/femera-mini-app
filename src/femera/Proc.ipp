#ifndef FEMERA_HAS_PROC_IPP
#define FEMERA_HAS_PROC_IPP

#include "../fmr/proc.hpp"

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
    return static_cast<Proc*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::init (int* argc, char** argv)
  noexcept {
    fmr::Exit_int err=0;
    try { Proc::derived (this)->task_init (argc, argv); }// Init this task,...
    catch (const Errs& e) { err = 1; e.print (); }
    catch (std::exception& e) { err = 2; }
    catch (...) { err = 3; }
    this->init_list (argc, argv);// ...then init the sub-tasks.
    this->set_base_n ();
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::exit (fmr::Exit_int err)
  noexcept {
    if (!this->task_list.empty()) {
      this->exit_tree ();// Exit the task tree below this (is noexcept), ...
    }
#if 0
    if (err>0) { return err; }
#endif
    fmr::Exit_int task_err =0;
    try { Proc::derived (this)->task_exit (); }//...then exit this derived task.
    catch (const Errs& e) { task_err = 1; e.print (); }
    catch (std::exception& e) { task_err = 2; }
    catch (...) { task_err = 3; }
    return (task_err > 0) ? task_err : err;
  }
  template <typename T> inline
  FMR_SMART_PTR<T> Proc<T>::get_task_spt (const fmr::Local_int i) {
    return std::static_pointer_cast<T> (this->get_work_spt (i));
  }
  template <typename T> inline
  FMR_SMART_PTR<T> Proc<T>::get_task_spt (const Work::Task_path_t path) {
    return std::static_pointer_cast<T> (this->get_work_spt (path));
  }
  template <typename T> inline
  T* Proc<T>::get_task_raw (const fmr::Local_int i) {
    return derived (this->get_work_raw (i));
  }
  template <typename T> inline
  T* Proc<T>::get_task_raw (const Work::Task_path_t path) {
    return derived (this->get_work_raw (path));
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Proc<T>::new_task ()
  noexcept {
    return FMR_MAKE_SMART<T> (T());
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Proc<T>::new_task (const Work::Core_ptrs core)
  noexcept {
    return std::move(FMR_MAKE_SMART<T> (T(core)));
  }
#if 0
  template <typename T> inline
  bool Proc<T>::is_main (bool ans=true) {// recursive version cannot inline
    ans = ans & (this->get_proc_ix () == this->main_ix);
    if (! this->task_list.empty ()) {
      ans = this->get_task_raw (0)->is_main (ans);
    }
    return ans;
  }
#else
  template <typename T> inline
  bool Proc<T>::is_main () { bool ans=true;// series version
    auto P = this;
    while (! P->task_list.empty ()) {
      ans = ans & (P->get_proc_ix () == P->main_ix);
      P = this_cast (P->get_work_raw (0));
    }
    return ans;
  }
#endif
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_id () {fmr::Local_int id=0;
    auto P = this;
    while (! P->task_list.empty ()) {
      id += P->base_id + P->base_n * P->get_proc_ix ();
#ifdef FMR_DEBUG
      printf ("%s id: %u += %u + %u * %u\n",
        P->name.c_str(), id, P->base_id, P->base_n, P->get_proc_ix ());
#endif
      P = this_cast (P->get_work_raw (0));
    }
    return id;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_ix ()
  noexcept {
    return this->proc_ix;
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
#if 0
  template <typename T> inline constexpr
  fmr::Local_int Proc<T>::get_node_n () {
    return fmr::Local_int (1);
  }
  template <typename T> inline constexpr
  fmr::Local_int Proc<T>::get_node_ix () {
    return fmr::Local_int (0);
  }
#endif
# if 0
  template <typename T> inline constexpr
  fmr::Local_int Proc<T>::get_node_core_n () {// physical cores
    return fmr::proc::get_node_core_n ();
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_hype_n () {// physical+logical cores
    return fmr::proc::get_node_hype_n ();
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_core_ix () {
    return fmr::proc::get_node_core_ix ();
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_hype_ix () {
    return fmr::proc::get_node_hype_ix ();
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_numa_ix () {
    return fmr::proc::get_node_numa_ix ();
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_numa_n () {
    return fmr::proc::get_node_numa_n ();
  }
  template <typename T> inline
  fmr::Global_int Proc<T>::get_used_byte () {
    return fmr::proc::get_used_byte ();
  }
#   endif
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_PROC_IPP
#endif