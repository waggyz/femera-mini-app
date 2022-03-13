#ifndef FEMERA_HAS_TEST_IPP
#define FEMERA_HAS_TEST_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  template <typename T> inline
  std::string Test<T>::get_base_name ()
  noexcept {
    return "test";
  }
  template <typename T> inline
  T* Test<T>::derived (Test* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  T* Test<T>::derived (Work* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Test<T>::init (int* argc, char** argv)
  noexcept {
    fmr::Exit_int err = 0;
    try { Test::derived (this)->task_init (argc, argv); }// Init this task,...
    catch (const Errs& e)    { err = 1; e.print (); }
    catch (std::exception& e){ err = 2; Errs::print (abrv+" task_init", e); }
    catch (...)              { err = 3; Errs::print (abrv+" task_exit"); }
    init_list (argc, argv);//                           then init these tasks.
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Test<T>::exit (const fmr::Exit_int err)
  noexcept {
    exit_list ();//                                         Exit these tasks,...
    fmr::Exit_int task_err = 0;
    try { Test::derived (this)->task_exit (); }// ...then try to exit this task.
    catch (const Errs& e)    { task_err = 1; e.print (); }
    catch (std::exception& e){ task_err = 2; Errs::print (abrv+" task_exit",e);}
    catch (...)              { task_err = 3; Errs::print (abrv+" task_exit"); }
    return (task_err > 0) ? task_err : err;
  }
  template <typename T> inline
  T* Test<T>::get_task (const fmr::Local_int i)
  noexcept {
    return derived (this->get_work_raw (i));
  }
  template <typename T> inline 
  T* Test<T>::get_task (Work::Task_path_t tree)
  noexcept {
    return derived (this->get_work_raw (tree));
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Test<T>::new_task (const Work::Core_ptrs_t core)
  noexcept {
    return FMR_MAKE_SMART(T) (T(core));
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_TEST_IPP
#endif