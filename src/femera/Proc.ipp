#ifndef FEMERA_HAS_PROC_IPP
#define FEMERA_HAS_PROC_IPP

#include "../fmr/proc.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T> inline
  T* Proc<T>::derived (Proc* ptr) {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::init (int* argc, char** argv) noexcept {
    fmr::Exit_int err=0;
    try { Proc::derived (this)->task_init (argc, argv); }// Init this task,
    catch (const Errs& e) { err = 1; e.print (); }
    catch (std::exception& e) { err = exit (1); }
    catch (...) { err = exit (2); }
    init_list (argc, argv);// then init the list.
#if 0
    this->proc_n = this->get_proc_n ();
#endif
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::exit (fmr::Exit_int err) noexcept {
    exit_tree ();   // Exit the task tree (is noexceptt),
    if (err>0) {return err;}// then exit this derived task.
    fmr::Exit_int task_err =0;
    try { Proc::derived (this)->task_exit (); }
    catch (const Errs& e) { task_err = 1; e.print (); }
    catch (std::exception& e) { task_err = 2; }
    catch (...) { task_err = exit (3); }
    return (task_err > 0) ? task_err : err;
  }
  template <typename T> inline
  std::shared_ptr<T> Proc<T>::get_task_spt (const fmr::Local_int i) {
    return std::static_pointer_cast<T> (this->get_work_spt (i));
  }
  template <typename T> inline
  std::shared_ptr<T> Proc<T>::get_task_spt (const Work::Task_path_t path) {
    return std::static_pointer_cast<T> (this->get_work_spt (path));
  }
  template <typename T> inline
  T* Proc<T>::get_task_raw (const fmr::Local_int i) {
    return static_cast<T*> (this->get_work_raw (i));
  }
  template <typename T> inline
  T* Proc<T>::get_task_raw (const Work::Task_path_t path) {
    return static_cast<T*> (this->get_work_raw (path));
  }
  template <typename T> inline constexpr
  std::shared_ptr<T> Proc<T>::new_task () noexcept {
    return std::make_shared<T> (T());
  }
  template <typename T> inline constexpr
  std::shared_ptr<T> Proc<T>::new_task (const Work::Core_t core) noexcept {
    return std::make_shared<T> (T(core));
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_ix () noexcept {
    return this->proc_ix;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_n () noexcept {
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
  proc::Team_t Proc<T>::get_team_id () noexcept {
    return this->team_id;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_team_n () noexcept {
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
  template <typename T> inline constexpr
  fmr::Local_int Proc<T>::get_node_core_n () {//NOTE physical+logical cores
    return fmr::proc::get_node_core_n ();
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_hype_n () {//NOTE physical+logical cores
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
  fmr::Global_int Proc<T>::get_node_used_byte () {
    return fmr::proc::get_node_used_byte ();
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_PROC_IPP
#endif