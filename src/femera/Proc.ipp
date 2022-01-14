#ifndef FEMERA_HAS_PROC_IPP
#define FEMERA_HAS_PROC_IPP


#include <thread>     // hardware_concurrency (ncpu)
//#include <sys/time.h> // rusage
#include <sys/resource.h> //rusage

#ifdef FMR_HAS_LIBNUMA
//FIXME Move to proc.cpp and put helper femera::proc:: functions there.
#include <numa.h>     // numa_node_of_cpu, numa_num_configured_nodes
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T> inline
  T* Proc<T>::derived (Proc* ptr) {
    return static_cast<T*> (ptr);
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::init (int* argc, char** argv) noexcept {
    fmr::Exit_int err=0;
    try { Proc::derived (this)->task_init (argc, argv); }// Init this task,
    catch (const Errs& e) { err = 1; e.print (); }
    catch (std::exception& e) { err = exit (1); }
    catch (...) { err = exit (2); }
    init_list (argc, argv);// then init the list.
#if 0
    this->proc_n = this->get_proc_n ();
#endif
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Proc<T>::exit (fmr::Exit_int err) noexcept {
    exit_tree ();   // Exit the task tree (is noexceptt),
    if (err>0) {return err;}// then exit this derived task.
    fmr::Exit_int task_err =0;
    try { Proc::derived (this)->task_exit (); }
    catch (const Errs& e) { task_err = 1; e.print (); }
    catch (std::exception& e) { task_err = 2; }
    catch (...) { task_err = exit (3); }
    return (task_err > 0) ? task_err : err;
  }
//FIXME Replace these =========================================================
# ifdef FMR_REMOVE_STUFF
  template <typename T> inline
  std::shared_ptr<T> Proc<T>::get_task (const fmr::Local_int i) {
    return std::static_pointer_cast<T> (this->get_work (i));
  }
  template <typename T> inline
  std::shared_ptr<T> Proc<T>::get_task (const Work::Task_path_t path) {
    return std::static_pointer_cast<T> (this->get_work (path));
  }
#   endif
//FIXME with these ------------------------------------------------------------
  template <typename T> inline
  T* Proc<T>::get_task_raw (const fmr::Local_int i) {
    return static_cast<T*> (this->get_work_raw (i));
  }
  template <typename T> inline
  T* Proc<T>::get_task_raw (const Work::Task_path_t path) {
    return static_cast<T*> (this->get_work_raw (path));
  }
  //===========================================================================
  template <typename T> inline constexpr
  std::shared_ptr<T> Proc<T>::new_task () noexcept {
    return std::make_shared<T> (T());
  }
  template <typename T> inline constexpr
  std::shared_ptr<T> Proc<T>::new_task (const Work::Core_t core) noexcept {
    return std::make_shared<T> (T(core));
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_ix () noexcept {
    return this->proc_ix;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_proc_n () noexcept {
#if 0
    this->proc_n =0;
    for (const auto W : this->task_list) {
      const auto P = static_cast<Proc<T>*> (W.get());
      this->proc_n += P->get_proc_n ();
    }
#endif
    return this->proc_n;
  }
  template <typename T> inline
  proc::Team_t Proc<T>::get_team_id () noexcept {
    return this->team_id;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_team_n () noexcept {
    return fmr::Local_int (this->task_list.size());
  }
#if 0
  template <typename T> inline constexpr
  fmr::Local_int Proc<T>::get_node_n () {
    return fmr::Local_int (1);
  }
  template <typename T> inline constexpr
  fmr::Local_int Proc<T>::get_node_ix () {
    return fmr::Local_int (0);
  }
#endif
  template <typename T> inline constexpr
  fmr::Local_int Proc<T>::get_node_core_n () {
#ifdef FMR_CORE_N
    return FMR_CORE_N ;
#else
    return Proc<T>::get_node_hype_n ();
#endif
  }
  template <typename T> inline constexpr
  fmr::Local_int Proc<T>::get_node_hype_n () {//NOTE physical+logical cores
    return fmr::Local_int (std::thread::hardware_concurrency());
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_core_ix () {
    return fmr::Local_int (fmr::Local_int (::sched_getcpu())
      % Proc<T>::get_node_core_n ());
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_hype_ix () {
    return fmr::Local_int (fmr::Local_int (::sched_getcpu()));
  }
#ifdef FMR_HAS_LIBNUMA
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_numa_ix () {
    if (::numa_available() != -1){return ::numa_node_of_cpu (::sched_getcpu());}
  return 0;
  }
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_numa_n () {
    if (::numa_available() != -1){return ::numa_num_configured_nodes(); }
  return 1;
  }
#else
  template <typename T> inline
  fmr::Local_int Proc<T>::get_node_numa_ix () {
    return fmr::Local_int (0);
  }
  template <typename T> inline constexpr
  fmr::Local_int Proc<T>::get_node_numa_n () {
    return fmr::Local_int (1);
  }
#endif
  template <typename T> inline
  fmr::Global_int Proc<T>::get_node_used_byte () {
    rusage r;
    getrusage (RUSAGE_SELF, & r);
    return fmr::Global_int (r.ru_maxrss * 1024);
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_PROC_IPP
#endif