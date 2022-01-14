#ifndef FEMERA_HAS_WORK_IPP
#define FEMERA_HAS_WORK_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  inline
  Work::Core_t Work::get_core ()
  noexcept {
    return std::make_tuple (this->proc, this->data, this->test);
  }
#if 1
//FIXME Replace these =========================================================
  inline
  Work_t Work::get_work (const fmr::Local_int i)
  noexcept {
    return (i < this->task_list.size()) ? this->task_list [i] : nullptr;
  }
  inline
  Work_t Work::get_work (const Work::Task_path_t path)
  noexcept {
    Work_t W = nullptr;
    const auto sz = path.size();
    if (sz > 0) {
      if (path[0] < this->get_task_n()) {
        W = this->get_work (path[0]);
        if (sz > 1) {
          for (fmr::Local_int i = 1; i < sz; i++) {
            if (W != nullptr) {
              if (path[i] < W->get_task_n()) { W = W->get_work (path[i]); }
              else { W = nullptr; }
    } } } } }
    return W;
  }
//FIXME with these ------------------------------------------------------------
#endif
  inline
  Work* Work::get_work_raw (const fmr::Local_int i)
  noexcept {
    return (i < this->task_list.size()) ? this->task_list [i].get() : nullptr;
  }
  inline
  Work* Work::get_work_raw (const Work::Task_path_t path)
  noexcept {
    Work* W = nullptr;
    const auto sz = path.size();
    if (sz > 0) {
      if (path[0] < this->get_task_n()) {
        W = this->get_work_raw (path[0]);
        if (sz > 1) {
          for (fmr::Local_int i = 1; i < sz; i++) {
            if (W != nullptr) {
              if (path[i] < W->get_task_n()) { W = W->get_work_raw (path[i]); }
              else { W = nullptr; }
    } } } } }
    return W;
  }
  //===========================================================================
  inline
  fmr::Local_int Work::add_task (Work_t W)
  noexcept {
    this->task_list.push_back (W);
    return fmr::Local_int (this->task_list.size () - 1);
  }
  inline
  fmr::Local_int Work::get_task_n ()
  noexcept {
    return fmr::Local_int (this->task_list.size ());
  }
  inline
  fmr::Exit_int Work::init_list (int* argc, char** argv)
  noexcept {fmr::Exit_int err =0;
    for (auto W : this->task_list) {// Init task_list forward.
      if (W != nullptr) {
#ifdef FMR_DEBUG
        printf ("Work: init list %s\n", W->name.c_str());
#endif
        const int Werr = W->init (argc, argv); // is noexcept
        err = (Werr == 0) ? err : Werr;
#if 0
        printf ("Work: init done %s\n", W->name.c_str());
#endif
  } }
  return err; }
}//end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_WORK_IPP
#endif