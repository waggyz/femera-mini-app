#ifndef FMR_HAS_WORK_IPP
#define FMR_HAS_WORK_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  inline Work::Work (Work* W)
    noexcept : proc (W->proc), file (W->file), data (W->data), test (W->test) {
  }
  inline Work_t Work::get_work (size_t i)
  noexcept {
    return (i < this->task_list.size ()) ? this->task_list [i] : nullptr;
  }
  inline size_t Work::add_task (Work_t W)
  noexcept {
    this->task_list.push_back (W);
    return this->task_list.size ();
  }
  inline size_t Work::get_task_n ()
  noexcept {
    return this->task_list.size ();
  }
  inline int Work::exit (int err) {
    while (! this->task_list.empty ()) {
      auto W = task_list.back ();// Exit in reverse order.
      if (W != nullptr) {//static_cast <Work_t> (nullptr)) {
	const auto Werr = W->exit (err);
	err = (Werr==0) ? err : Werr;
      }
      this->task_list.pop_back ();
    }
    return err;
  }
}// end femera:: namespace

#undef FMR_DEBUG
//end FMR_HAS_WORK_IPP
#endif