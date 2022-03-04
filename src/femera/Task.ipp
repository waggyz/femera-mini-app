#ifndef FEMERA_HAS_SIMS_IPP
#define FEMERA_HAS_SIMS_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T> inline
  T* Task<T>::derived (Task* ptr) {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Task<T>::init (int* argc, char** argv) noexcept {
    fmr::Exit_int err=0;
    try { Task::derived (this)->task_init (argc, argv); }// Init this task,
    catch (const Errs& e) { err = 1; e.print (); }
    catch (std::exception& e) { err = 2;
      femera::Errs::print (this->abrv+" task_init", e); }
    catch (...) { err = 3; }
    if (err > 0 ){ return this->exit (err); }
    this->init_list (argc, argv);// then init the list; is noexcept
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Task<T>::exit (fmr::Exit_int err) noexcept {
    fmr::Exit_int task_err =0;
    this->exit_list ();// is noexcept
    try { Task::derived (this)->task_exit (); }
    catch (const Errs& e) { task_err = 1; e.print (); }
    catch (std::exception& e) { err = 2;
      femera::Errs::print (this->abrv+" task_exit", e); }
    catch (...) { task_err = 3; }
    if (Task::derived (this)->do_exit_zero) { return 0; }
    return (task_err > 0) ? task_err : err;
  }
  template <typename T> inline
  std::string Task<T>::get_base_name () noexcept {
    return "sims";
  }
  template <typename T> inline
  T* Task<T>::get_task (const fmr::Local_int i) {
    return static_cast<T*> (this->get_work_raw (i));
  }
  template <typename T> inline
  T* Task<T>::get_task (const Work::Task_path_t path) {
    return static_cast<T*> (this->get_work_raw (path));
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Task<T>::new_task () noexcept {
    return FMR_MAKE_SMART(T) (T());
  }
  template <typename T> inline
  FMR_SMART_PTR<T> Task<T>::new_task (int* argc, char** argv) noexcept {
    auto S = FMR_MAKE_SMART(T) (T());
    S->init (argc, argv);
    return S;
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_SIMS_IPP
#endif