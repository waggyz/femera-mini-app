#ifndef FEMERA_HAS_DATA_IPP
#define FEMERA_HAS_DATA_IPP

#include "proc/Main.hpp"
#include "data/File.hpp"// circular reference ok for CRTP?

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  template <typename T> inline
  T* Data<T>::derived (Data* ptr) noexcept {
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
    std::vector<char> buf (w + 1, 0);
    std::snprintf (&buf[0], buf.size(), form.c_str(), args...);
    const auto line = std::string (&buf[0]);
    if (file != nullptr) { fprintf (file,"%s\n", line.c_str()); }
    return line;
  }
  template <typename T> template <typename ...Args> inline
  std::string Data<T>::text_line (const std::string& form, Args ...args) {
    return Data<T>::text_line (Data::File_ptrs_t ({}), form, args...);
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
    const auto line = femera::form::head_line (h, w, head, form, args...);
    if (file != nullptr) { fprintf (file,"%s\n", line.c_str()); }
    return line;
  }
  template <typename T> template <typename ...Args> inline
  std::string Data<T>::head_line
  (const std::string& head, const std::string& form, Args ...args) {
    return Data<T>::head_line (Data::File_ptrs_t ({}), head, form, args...);
  }
  template <typename T> template <typename ...Args> inline
  std::string Data<T>::head_time (const Data::File_ptrs_t flist,
    const std::string& head, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size ()];
    }
    const auto h = this->file_head_sz [file];
    const auto w = this->file_line_sz [file];
    const auto line = femera::form::head_time (h, w, head, form, args...);
    if (file != nullptr) { fprintf (file,"%s\n", line.c_str()); }
    return line;
  }
  template <typename T> template <typename ...Args> inline
  std::string Data<T>::head_time
  (const std::string& head, const std::string& form, Args ...args) {
    return Data<T>::head_time (Data::File_ptrs_t ({}), head, form, args...);
  }
  template <typename T> inline
  fmr::Exit_int Data<T>::init (int* argc, char** argv) noexcept {
    fmr::Exit_int err=0;
    try { Data::derived(this)->task_init (argc, argv); }// Init this task,...
    catch (const Errs& e) { err = 1; e.print (); }
    catch (std::exception& e) { err = 2;
      femera::Errs::print (this->abrv+" task_init", e); }
    catch (...) { err = 3;
      femera::Errs::print (this->abrv+" task_exit"); }
    init_list (argc, argv);//                     ...then init its task list.
    return err;
  }
  template <typename T> inline
  fmr::Exit_int Data<T>::exit (fmr::Exit_int err) noexcept {
    this->data = nullptr;
    if (err>0) { return err; }
    Work::exit_list ();       //  Exit this task list (exceptions caught),...
    if (err>0) { return err; }//                      ...then exit this task.
    try { Data::derived(this)->task_exit (); }
    catch (const Errs& e) { err = 1; e.print (); }
    catch (std::exception& e) { err = 2;
      femera::Errs::print (this->abrv+" task_exit", e); }
    catch (...) { err = 3;
      femera::Errs::print (this->abrv+" task_exit"); }
    return err;
  }
  template <typename T> inline
  std::string Data<T>::get_base_name () noexcept {
    return "data";
  }
  template <typename T> inline
  T* Data<T>::get_task (const fmr::Local_int i) noexcept {
    return static_cast<T*> (this->get_work_raw (i));
  }
  template <typename T> inline
  T* Data<T>::get_task (const Work::Task_path_t tree) noexcept {
    return static_cast<T*> (this->get_work_raw (tree));
  }
  template <typename T> inline constexpr
  FMR_SMART_PTR<T> Data<T>::new_task (const Work::Core_ptrs_t core) noexcept {
    return FMR_MAKE_SMART(T) (T(core));
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_DATA_IPP
#endif