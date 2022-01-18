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
  inline
  Work_t Work::get_work_spt (const fmr::Local_int i)
  noexcept {
    return (i < this->task_list.size()) ? this->task_list [i] : nullptr;
  }
  inline
  Work_t Work::get_work_spt (const Work::Task_path_t path)
  noexcept {
    Work_t W = nullptr;
    auto Wraw = this;
    const auto sz = path.size();
#if 0
    if (sz > 0) {
      if (path[0] < this->get_task_n()) {
        W = this->get_work_spt (path[0]);
        if (sz > 1) {
          for (fmr::Local_int i = 1; i < sz; i++) {
            if (W != nullptr) {
              if (path[i] < W->get_task_n()) { W = W->get_work_spt (path[i]); }
              else { W = nullptr; }
    } } } } }
#else
    if (sz > 0) {
      for (fmr::Local_int i = 0; i < sz; i++) {
        if (Wraw != nullptr) {
          W = W->get_work_spt (path[i]);
          Wraw = W.get();
    } } }
#endif
    return W;
  }
  inline
  Work* Work::get_work_raw (const fmr::Local_int i)
  noexcept {
    return (i < this->task_list.size()) ? this->task_list [i].get() : nullptr;
  }
  inline
  Work* Work::get_work_raw (const Work::Task_path_t path)
  noexcept {
    auto W = this;
    const auto sz = path.size ();
    if (sz > 0) {
      for (fmr::Local_int i = 0; i < sz; i++) {
        if (W != nullptr) {
          W = W->get_work_raw (path[i]);
    } } }
#if 0
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
#endif
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
}//end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_WORK_IPP
#endif