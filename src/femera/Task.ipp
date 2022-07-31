#ifndef FEMERA_HAS_TASK_IPP
#define FEMERA_HAS_TASK_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T> inline
  Task<T>::Task (const Work::Core_ptrs_t W) noexcept : Work (W) {
  }
  template <typename T> inline
  std::string Task<T>::get_base_abrv ()
  noexcept {
    return "task";
  }
  template <typename T> inline
  fmr::Exit_int Task<T>::init (int* argc, char** argv)
  noexcept {
    fmr::Exit_int err = 0;
    try { Task::this_cast (this)->task_init (argc, argv); }// Init this task,...
    catch (const Warn& e)    { err =-1; e.print (); }
 //   catch (const Note& e)    { err = 0; e.print (); }//TODO
    catch (const Errs& e)    { err = 1; e.print (); }
    catch (std::exception& e){ err = 2; Errs::print (this->get_abrv ()+" task_init", e); }
    catch (...)              { err = 3; Errs::print (abrv+" task_init"); }
    if (err > 0) { return this->exit (err); }
    const auto list_err = Work::init_list (argc, argv);// ...then init children.
    return (list_err > 0) ? list_err : err;
  }
  template <typename T> inline
  fmr::Exit_int Task<T>::exit (fmr::Exit_int err)
  noexcept {
    const auto list_err = Work::exit_list ();//        Exit child tasks,...
    err = (list_err > 0) ? list_err : err;
    fmr::Exit_int task_err = 0;
    try { Task::this_cast (this)->task_exit (); }// ...then exit this task.
    catch (const Warn& e)    { task_err =-1; e.print (); }
//    catch (const Note& e)    { task_err = 0; e.print (); }//TODO
    catch (const Errs& e)    { task_err = 1; e.print (); }
    catch (std::exception& e){ task_err = 2; Errs::print (abrv+" task_exit",e);}
    catch (...)              { task_err = 3; Errs::print (abrv+" task_exit"); }
    if (this->do_exit_zero) { return 0; }
    return (task_err > 0) ? task_err : err;
  }
  template <typename T> inline
  FMR_SMART_PTR<T> Task<T>::new_task (int* argc, char** argv)
  noexcept {
    auto S = FMR_MAKE_SMART(T) (T());
    S->init (argc, argv);
    return S;
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Task<T>::new_task ()
  noexcept {
    return FMR_MAKE_SMART(T) (T());
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Task<T>::new_task (const Work::Core_ptrs_t core)
  noexcept {
    return FMR_MAKE_SMART(T) (T(core));
  }
  template <typename T> inline
  T* Task<T>::get_task (const fmr::Local_int i)
  noexcept {
    return static_cast<T*> (this->get_work (i));
  }
  template <typename T> inline
  T* Task<T>::get_task (const Work::Task_path_t path)
  noexcept {
    return static_cast<T*> (this->get_work (path));
  }
  template <typename T> inline constexpr
  T* Task<T>::this_cast (Task* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline constexpr
  T* Task<T>::this_cast (Work* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_TASK_IPP
#endif