#ifndef FEMERA_HAS_PHYS_IPP
#define FEMERA_HAS_PHYS_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  template <typename T> inline
  Phys<T>::Phys (const Work::Core_ptrs_t W) noexcept : Work (W) {
  }
  template <typename T> inline
  std::string Phys<T>::get_base_abrv ()
  noexcept {
    return "phys";
  }
  template <typename T> inline constexpr
  T* Phys<T>::this_cast (Phys* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline constexpr
  T* Phys<T>::this_cast (Work* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Phys<T>::init (int* argc, char** argv)
  noexcept {
    fmr::Exit_int err = 0;
    try { Phys::this_cast (this)->task_init (argc, argv); }// Init this task,...
    catch (const Warn& e)    { err =-1; e.print (); }
    catch (const Errs& e)    { err = 1; e.print (); }
    catch (std::exception& e){ err = 2;
      Errs::print (this->get_abrv ()+" task_init", e); }
    catch (...)              { err = 3;
      Errs::print (this->get_abrv ()+" task_init"); }
FMR_WARN_INLINE_OFF
    if (err > 0) { return this->exit (err); }
FMR_WARN_INLINE_ON
    Work::init_list (argc, argv);//                       then init child tasks.
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Phys<T>::exit (const fmr::Exit_int err)
  noexcept {
    Work::exit_list ();//                                 Exit child tasks,...
    fmr::Exit_int task_err = 0;
    try { Phys::this_cast (this)->task_exit (); }// ...then try to exit this task.
    catch (const Warn& e)    { task_err =-1; e.print ();}
    catch (const Errs& e)    { task_err = 1; e.print ();}
    catch (std::exception& e){ task_err = 2;
    Errs::print (this->get_abrv ()+" task_exit", e);}
    catch (...)              { task_err = 3;
    Errs::print (this->get_abrv ()+" task_exit");}
    this->set_init (false);
    return (task_err > 0) ? task_err : err;
  }
  template <typename T> inline
  T* Phys<T>::get_task (const fmr::Local_int i)
  noexcept {
    return this_cast (this->get_work (i));
  }
  template <typename T> inline
  T* Phys<T>::get_task (Work::Task_path_t tree)
  noexcept {
    return this_cast (this->get_work (tree));
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Phys<T>::new_task (const Work::Core_ptrs_t core)
  noexcept {
    return FMR_MAKE_SMART(T) (T(core));
  }
  template <typename T> inline
  T* Phys<T>::get_task (const Work_type t, const fmr::Local_int ix)
  noexcept {
    return Phys::this_cast (Work::get_work (t, ix));
  }
  template <typename T> inline
  T* Phys<T>::get_task (const Task_type t, const fmr::Local_int ix)
  noexcept {
    return Phys::this_cast (Work::get_work (task_cast (t), ix));
  }//
  //
}// end femera:: namespace
//
#undef FMR_DEBUG
//end FEMERA_HAS_PHYS_IPP
#endif
