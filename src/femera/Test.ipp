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
    catch (const std::exception& e) { err = exit (2); }
    catch (...) { err = exit (2); }
    init_list (argc, argv);// is noexcept
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Test<T>::exit (fmr::Exit_int err) noexcept {
    if (err>0) {return err;}
    exit_list ();// is noexcept
    if (err>0) {return err;}// then exit this task.
    try { Test::derived (this)->task_exit (); }
    catch (const std::exception& e) { err = 2; }
    catch (...) { err = exit (2); }
    return err;
  }
  template <typename T> inline
  T* Test<T>::get_task (const fmr::Local_int i) {
    return derived (this->get_work_raw (i));
  }
  template <typename T> inline 
  T* Test<T>::get_task (Work::Task_path_t tree) {
    return derived (this->get_work_raw (tree));
  }
#if 0
  template <typename T> inline constexpr
  std::shared_ptr<T> Test<T>::new_task () noexcept {
    return std::make_shared<T> (T());
  }
#endif
  template <typename T> inline constexpr
  std::shared_ptr<T> Test<T>::new_task (const Work::Core_t core) noexcept {
    return std::make_shared<T> (T(core));
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_TEST_IPP
#endif