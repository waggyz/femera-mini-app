#ifndef FEMERA_HAS_TEST_IPP
#define FEMERA_HAS_TEST_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  template <typename T> inline
  T* Test<T>::derived (Test* ptr) {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Test<T>::init (int* argc, char** argv) noexcept {
    fmr::Exit_int err=0;
    try { Test::derived(this)->task_init (argc, argv); }// Init this task,
    catch (std::exception& e) { err = exit (2); }
    try { init_list (argc, argv); }                     // then init the list.
    catch (std::exception& e) { err = exit (1); }
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Test<T>::exit (fmr::Exit_int err) noexcept {
    if (err>0) {return err;}
    err = exit_list ();// Exit the task stack (exceptions caught),
    if (err>0) {return err;}// then exit this task.
    try { Test::derived(this)->task_exit (); }
    catch (std::exception& e) { err = 2; }
    return err;
  }
  template <typename T> inline
  std::shared_ptr<T> Test<T>::get_task (const fmr::Local_int i) {
    return std::static_pointer_cast<T> (this->get_work (i));
  }
  template <typename T> inline
  std::shared_ptr<T> Test<T>::get_task (Work::Path_t tree) {
    return std::static_pointer_cast<T> (this->get_work (tree));
  }
  template <typename T> inline constexpr
  std::shared_ptr<T> Test<T>::new_task () noexcept {
    return std::make_shared<T> (T());
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_TEST_IPP
#endif