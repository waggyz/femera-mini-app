#ifndef FEMERA_HAS_PROC_IPP
#define FEMERA_HAS_PROC_IPP

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
    try { Proc::derived(this)->task_init (argc, argv); }// Init this task,
    catch (std::exception& e) { err = exit (2); }
    try { init_list (argc, argv); }                     // then init the list.
    catch (std::exception& e) { err = exit (1); }
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::exit (fmr::Exit_int err) noexcept {
    if (err>0) {return err;}
    err = exit_tree ();     // Exit the task tree (exceptions caught),
    if (err>0) {return err;}// then exit this derived task.
    try { Proc::derived(this)->task_exit (); }
    catch (std::exception& e) { err = 2; }
    return err;
  }
  template <typename T> inline
  std::shared_ptr<T> Proc<T>::get_task (const fmr::Local_int i) {
    return std::static_pointer_cast<T> (this->get_work (i));
  }
  template <typename T> inline
  std::shared_ptr<T> Proc<T>::get_task (const Work::Task_path_t path) {
    return std::static_pointer_cast<T> (this->get_work (path));
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
  proc::Team_t Proc<T>::get_team_id () noexcept {
    return Proc::derived(this)->get_team_id ();
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_PROC_IPP
#endif