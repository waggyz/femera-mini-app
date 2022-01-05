#ifndef FEMERA_HAS_SIMS_IPP
#define FEMERA_HAS_SIMS_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T> inline
  T* Sims<T>::derived (Sims* ptr) {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Sims<T>::init (int* argc, char** argv) noexcept {
    fmr::Exit_int err=0;
    try { Sims::derived(this)->task_init (argc, argv); }// Init this task,
    catch (std::exception& e) { err = exit (2); }
    try { init_list (argc, argv); }                     // then init the list.
    catch (std::exception& e) { err = exit (1); }
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Sims<T>::exit (fmr::Exit_int err) noexcept {
//    if (err>0) {return err;}
    err = exit_list ();
//    if (err>0) {return err;}// then exit this derived task.
    try { Sims::derived(this)->task_exit (); }
    catch (std::exception& e) { err = 2; }
    return err;
  }
  template <typename T> inline
  std::shared_ptr<T> Sims<T>::get_task (const fmr::Local_int i) {
    return std::static_pointer_cast<T> (this->get_work (i));
  }
  template <typename T> inline
  std::shared_ptr<T> Sims<T>::get_task (const Work::Task_path_t path) {
    return std::static_pointer_cast<T> (this->get_work (path));
  }
  template <typename T> inline constexpr
  std::shared_ptr<T> Sims<T>::new_task () noexcept {
    return std::make_shared<T> (T());
  }
  template <typename T> inline
  std::shared_ptr<T> Sims<T>::new_task (int* argc, char** argv) noexcept {
    auto S = std::make_shared<T> (T());
    S->init (argc, argv);
    return S;
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_SIMS_IPP
#endif