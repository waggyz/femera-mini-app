#ifndef FEMERA_HAS_DATA_IPP
#define FEMERA_HAS_DATA_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  template <typename T> inline
  Data<T>::Data (const Work::Core_ptrs_t core)
  noexcept : Work (core) {
  }
  template <typename T> inline
  std::string Data<T>::get_base_abrv ()
  noexcept {
    return "data";
  }
  template <typename T> inline constexpr
  T* Data<T>::this_cast (Data* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline constexpr
  T* Data<T>::this_cast (Work* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Data<T>::init (int* argc, char** argv)
  noexcept {
#if 1//def FMR_DEBUG
    printf ("%4s %4s %4s %s\n",get_base_abrv ().c_str(), get_abrv ().c_str(),
      "init", "trying task_init (..)..." );
#endif
    fmr::Exit_int err = 0;
    try { Data::this_cast (this)->task_init (argc, argv); }// Init this task,...
    catch (const Warn& e)    { err =-1; e.print (); }
    catch (const Errs& e)    { err = 1; e.print (); }
    catch (std::exception& e){ err = 2;
      Errs::print (this->get_abrv ()+" task_init", e); }
    catch (...)              { err = 3;
      Errs::print (this->get_abrv ()+" task_init"); }
    if (err > 0) {return this->exit (err); }
    err = Work::init_list (argc, argv);//              ...then init child tasks.
#ifdef FMR_DEBUG
    const auto n = this->get_task_n ();
    if (n>0) {
      auto msg = std::string("data init mods ");
        for (fmr::Local_int i=0; i<n; ++i) {
          msg += this->get_task (i)->abrv;
          msg += (i == (n - 1)) ? "\n":" ";
      }
    printf (msg.c_str());
    }
#endif
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Data<T>::exit (const fmr::Exit_int err)
  noexcept {
    Work::exit_list ();//              Exit child tasks (exceptions caught),...
    this->data = nullptr;
    fmr::Exit_int task_err = 0;
    try  { Data::this_cast (this)->task_exit (); }//    ...then exit this task.
    catch (const Warn& e)    { task_err =-1; e.print (); }
    catch (const Errs& e)    { task_err = 1; e.print (); }
    catch (std::exception& e){ task_err = 2;
      Errs::print (this->get_abrv ()+" task_exit",e);}
    catch (...)              { task_err = 3;
      Errs::print (this->get_abrv ()+" task_exit"); }
    return (task_err > 0) ? task_err : err;
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Data<T>::new_task (const Work::Core_ptrs_t core)
  noexcept {
    return FMR_MAKE_SMART(T) (T(core));
  }
  template <typename T> inline
  T* Data<T>::get_task (const fmr::Local_int i)
  noexcept {
    return static_cast<T*> (this->get_work (i));
  }
  template <typename T> inline
  T* Data<T>::get_task (const Work::Task_path_t tree)
  noexcept {
    return static_cast<T*> (this->get_work (tree));
  }
  template <typename T> inline
  T* Data<T>::get_task (const Work_type t, const fmr::Local_int ix)
  noexcept {
    return Data::this_cast (Work::get_work (t, ix));
  }
  template <typename T> inline
  T* Data<T>::get_task (const Task_type t, const fmr::Local_int ix)
  noexcept {
    return Data::this_cast (Work::get_work (task_cast (t), ix));
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_DATA_IPP
#endif
