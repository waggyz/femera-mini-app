#ifndef FEMERA_HAS_DATA_IPP
#define FEMERA_HAS_DATA_IPP

#include "core.h"

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
  std::string Data<T>::text_line
  (const Data::File_ptrs_t f, const std::string& form,...){
    std::string line = Data<T>::text_line (form);
    if (f.size () > 0 && this->proc != nullptr) {
      const auto fh = f[this->proc->get_proc_id () % f.size()];
      if (fh != nullptr) {
        fprintf (fh,"%s\n", line.c_str());
    } }
    return line;
  }
  template <typename T> inline
  std::string Data<T>::text_line (const std::string& form,...){
    std::string line = form;// TODO fmr::form::text_line (..)
    return line;
  }
  template <typename T> inline
  std::string Data<T>::head_line
  (Data::File_ptrs_t f, const std::string& head, const std::string& form,...){
    std::string line = Data<T>::head_line (head, form);
    if (f.size () > 0 && this->proc != nullptr) {
      const auto fh = f[this->proc->get_proc_id () % f.size()];
      if (fh != nullptr) {
        fprintf (fh,"%s\n", line.c_str());
    } }
    return line;
  }
  template <typename T> inline
  std::string Data<T>::head_line
  (const std::string& head, const std::string& form,...){
    std::string line = head+" "+form;// TODO fmr::form::head_line (..)
    return line;
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
  FMR_SMART_PTR<T> Data<T>::get_task_spt (const fmr::Local_int i) {
    return std::static_pointer_cast<T> (this->get_work_spt (i));
  }
  template <typename T> inline
  FMR_SMART_PTR<T> Data<T>::get_task_spt (const Work::Task_path_t tree) {
    return std::static_pointer_cast<T> (this->get_work_spt (tree));
  }
  template <typename T> inline
  T* Data<T>::get_task_raw (const fmr::Local_int i) {
    return static_cast<T*> (this->get_work_raw (i));
  }
  template <typename T> inline
  T* Data<T>::get_task_raw (const Work::Task_path_t tree) {
    return static_cast<T*> (this->get_work_raw (tree));
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Data<T>::new_task (const Work::Core_ptrs core) noexcept {
    return FMR_MAKE_SMART<T> (T(core));
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_DATA_IPP
#endif