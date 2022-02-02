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
  template <typename T> template <typename ...Args> inline
  std::string Data<T>::text_line
  (const Data::File_ptrs_t flist, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size()];
    }
    const auto w = this->file_line_sz [file];
    std::vector<char> buf (w + 1, 0);//TODO line = fmr::form::text_line (..)
    std::snprintf (&buf[0], w, form.c_str(), args...);
    const auto line = std::string(&buf[0]);
    if (file != nullptr) {
      fprintf (file,"%s\n", line.c_str());
    }
    return line;
  }
  template <typename T> template <typename ...Args> inline
  std::string Data<T>::text_line (const std::string& form, Args ...args) {
    return Data<T>::text_line (Data::File_ptrs_t({nullptr}), form, args...);
  }
  template <typename T> template <typename ...Args> inline
  std::string Data<T>::head_line (const Data::File_ptrs_t flist,
    const std::string& head, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size()];
    }
    const auto w = this->file_line_sz [file];
    const auto h = this->file_head_sz [file];
    std::vector<char> buf (w + 1, 0);//TODO line = fmr::form::head_line (..)
    const auto format = "%" + std::to_string(h) +"s "+ form;
    std::snprintf (&buf[0], w, format.c_str(), head.c_str(), args...);
    const auto line = std::string(&buf[0]);
    if (file != nullptr) {
      fprintf (file,"%s\n", line.c_str());
    }
    return line;
  }
  template <typename T> template <typename ...Args> inline
  std::string Data<T>::head_line
  (const std::string& head, const std::string& form, Args ...args) {
    return Data<T>::head_line (Data::File_ptrs_t({nullptr}),
      head, form, args...);
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
    this->data = nullptr;
    if (err>0) {return err;}
    Work::exit_list ();// Exit this task stack (exceptions caught),
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