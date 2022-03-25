#ifndef FEMERA_HAS_PROC_IPP
#define FEMERA_HAS_PROC_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T> inline
  std::string Proc<T>::get_base_name ()
  noexcept {
    return "proc";
  }
  template <typename T> inline constexpr
  T* Proc<T>::this_cast (Proc* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline constexpr
  T* Proc<T>::this_cast (Work* ptr)
  noexcept {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::init (int* argc, char** argv)
  noexcept {
    fmr::Exit_int err = 0;
    try { Proc::this_cast(this)->task_init (argc, argv); }// Init this task,...
    catch (const Warn& e)    { err =-1; e.print (); }
    catch (const Errs& e)    { err = 1; e.print (); }
    catch (std::exception& e){ err = 2;
    Errs::print (this->get_abrv ()+" task_init", e); }
    catch (...)              { err = 3;
    Errs::print (this->get_abrv ()+" task_init"); }
    if (err > 0) {return this->exit (err); }
    Work::init_list (argc, argv);//                    ...then init child tasks.
    this->set_base_n ();// Set values for calculating proc_id.
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::exit (const fmr::Exit_int err)
  noexcept {
    this->data = nullptr;
    Work::exit_tree ();//                     Exit the task tree below this,...
    fmr::Exit_int task_err = 0;
    try  { Proc::this_cast (this)->task_exit (); }// ...then try to exit this task.
    catch (const Warn& e)    { task_err =-1; e.print (); }
    catch (const Errs& e)    { task_err = 1; e.print (); }
    catch (std::exception& e){ task_err = 2;
    Errs::print (this->get_abrv ()+" task_exit",e);}
    catch (...)              { task_err = 3;
    Errs::print (this->get_abrv ()+" task_exit"); }
    return (task_err > 0) ? task_err : err;
  }
  template <typename T> inline
  T* Proc<T>::get_task (const fmr::Local_int i)
  noexcept {
    return Proc::this_cast (Work::get_work_raw (i));
  }
  template <typename T> inline
  T* Proc<T>::get_task (const Work::Task_path_t path)
  noexcept {
    return Proc::this_cast (Work::get_work_raw (path));
  }
  template <typename T> inline
  T* Proc<T>::get_task (const Task_type t, const fmr::Local_int ix)
  noexcept {
    return Proc::this_cast (Work::get_work_raw (t, ix));
  }
  template <typename T> inline
  T* Proc<T>::get_task (const Plug_type t, const fmr::Local_int ix)
  noexcept {
    return Proc::this_cast (Work::get_work_raw (task_cast (t), ix));
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Proc<T>::new_task ()
  noexcept {
    return FMR_MAKE_SMART(T) (T());
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Proc<T>::new_task (const Work::Core_ptrs_t core)
  noexcept {
#if 0
    return std::move(FMR_MAKE_SMART<T> (T(core)));
#else
    return FMR_MAKE_SMART(T) (T(core));
#endif
  }
#if 0
  template <typename T> inline
  bool Proc<T>::is_main (bool ans=true) {// recursive version does not inline
    ans = ans & (this->get_proc_ix () == this->main_ix);
    if (! this->task_list.empty ()) {
      ans = this->get_task (0)->is_main (ans);
    }
    return ans;
  }
#else
  template <typename T> inline
  bool Proc<T>::is_main ()// series version
  noexcept { bool ans=true;
    Proc* P = this;
    ans = ans & (P->get_proc_ix () == P->main_ix);
    while (! P->task_list.empty ()) {
      P = static_cast<Proc*> (P->get_work_raw (0));//TODO only path 0? 
      ans = ans & (P->get_proc_ix () == P->main_ix);
    }
    return ans;
  }
#endif
#if 0
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_id () {fmr::Local_int id=0;
    Proc* P = this;
    id += P->base_id + P->base_n * P->get_proc_ix ();
    while (! P->task_list.empty ()) {
      P = static_cast<Proc*> (P->get_work_raw (0));
      id += P->base_id + P->base_n * P->get_proc_ix ();
#ifdef FMR_DEBUG
      printf ("%s id: %u += %u + %u * %u\n", P->get_abrv ().c_str(), id,
        P->base_id, P->base_n, P->get_proc_ix ());
#endif
    }
    return id;
  }
#else
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_id (fmr::Local_int id)
  noexcept {
    const fmr::Local_int tid
      = this->base_id + this->base_n * Proc::this_cast (this)->get_proc_ix ();
    if (! this->task_list.empty ()) {
      id += this->get_task (0)->get_proc_id (tid);//TODO only path 0?
#ifdef FMR_DEBUG
      printf ("%s id: %u += %u + %u * %u\n", this->get_abrv ().c_str(), id,
        this->base_id, this->base_n, Proc::this_cast (this)->get_proc_ix ());
#endif
    }
    return id;
  }
#endif
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_ix ()
  noexcept {
#ifdef FMR_DEBUG
    printf ("Proc<%s>::get_proc_ix: %u\n", this->get_abrv ().c_str(),
      Proc::this_cast (this)->task_proc_ix ());
#endif
    return Proc::this_cast (this)->task_proc_ix ();
    //NOTE proc->task_proc_ix() always calls Main::task_proc_ix ().
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_n ()
  noexcept {
    return this->proc_n;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::all_proc_n ()
  noexcept {fmr::Local_int n=1;
    auto P = this;
    while (! P->task_list.empty ()) {
      n *= P->get_proc_n ();
#ifdef FMR_DEBUG
      printf ("%s: * %u = %u\n", P->get_name ().c_str(), P->get_proc_n(), n);
#endif
      P = this_cast (P->get_work_raw (0));//TODO other branches?
    }
    return n;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::set_base_n () {
    auto P = this;
    while (! P->task_list.empty ()) {
      P->base_n = P->all_proc_n () / P->proc_n;
      P = this_cast (P->get_work_raw (0));//TODO other branches?
    }
    return this->base_n;
  }
  template <typename T> inline
  proc::Team_t Proc<T>::get_team_id ()
  noexcept {
    return this->team_id;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_team_n ()
  noexcept {
    return fmr::Local_int (this->task_list.size());
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_PROC_IPP
#endif