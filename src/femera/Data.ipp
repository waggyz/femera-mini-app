#ifndef FEMERA_HAS_DATA_IPP
#define FEMERA_HAS_DATA_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  template <typename T> inline
  T* Data<T>::derived (Data* ptr) {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Data<T>::init (int* argc, char** argv) noexcept {
    fmr::Exit_int err=0;
    try { Data::derived(this)->task_init (argc, argv); }// Init this task,
    catch (std::exception& e) { err = exit (2); }
    catch (...) { err = this->exit (2); }
    init_list (argc, argv);// then init its list.
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Data<T>::exit (fmr::Exit_int err) noexcept {
    if (err>0) {return err;}
    exit_list ();// Exit this task stack (exceptions caught),
    if (err>0) {return err;}// then exit this task.
    try { Data::derived(this)->task_exit (); }
    catch (std::exception& e) { err = 2; }
    catch (...) { err = 2; }
    return err;
  }
  template <typename T> inline
  std::shared_ptr<T> Data<T>::get_task (const fmr::Local_int i) {
    return std::static_pointer_cast<T> (this->get_work (i));
  }
  template <typename T> inline
  std::shared_ptr<T> Data<T>::get_task (const Work::Task_path_t tree) {
    return std::static_pointer_cast<T> (this->get_work (tree));
  }
  template <typename T> inline constexpr
  std::shared_ptr<T> Data<T>::new_task (const Work::Core_t core) noexcept {
    return std::make_shared<T> (T(core));
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_DATA_IPP
#endif