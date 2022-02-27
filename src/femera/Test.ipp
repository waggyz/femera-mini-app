#ifndef FEMERA_HAS_TEST_IPP
#define FEMERA_HAS_TEST_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  template <typename T> inline
  T* Test<T>::derived (Test* ptr) noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  T* Test<T>::derived (Work* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Test<T>::init (int* argc, char** argv) noexcept {
    fmr::Exit_int err=0;
    try { Test::derived (this)->task_init (argc, argv); }// Init this task,
    catch (std::exception& e) { err = 2;
      femera::Errs::print (this->abrv+" task_init", e); }
    catch (...) { err = exit (3); }
    init_list (argc, argv);// is noexcept
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Test<T>::exit (fmr::Exit_int err) noexcept {
    if (err>0) {return err;}
    exit_list ();// is noexcept
    if (err>0) {return err;}// then exit this task.
    try { Test::derived (this)->task_exit (); }
    catch (std::exception& e) { err = 2;
      femera::Errs::print (this->abrv+" task_exit", e); }
    catch (...) { err = exit (3); }
    return err;
  }
  template <typename T> inline
  std::string Test<T>::get_base_name () noexcept {
    return "test";
  }
  template <typename T> inline
  T* Test<T>::get_task (const fmr::Local_int i) {
    return derived (this->get_work_raw (i));
  }
  template <typename T> inline 
  T* Test<T>::get_task (Work::Task_path_t tree) {
    return derived (this->get_work_raw (tree));
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Test<T>::new_task (const Work::Core_ptrs_t core) noexcept {
    return FMR_MAKE_SMART<T> (T(core));
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_TEST_IPP
#endif