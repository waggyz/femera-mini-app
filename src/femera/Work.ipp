#ifndef FMR_HAS_WORK_IPP
#define FMR_HAS_WORK_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
#if 0
  inline Work::Work (Work* W)
    noexcept : proc (W->proc), file (W->file), data (W->data), test (W->test) {
  }
  inline Work::Work (Make_work_t W)
    noexcept {
    std::tie(this->proc,this->file,this->data, this->test) = W;
  }
#endif
  inline Work::Make_work_t Work::ptrs ()
    noexcept {
    return std::make_tuple(this->proc,this->file, this->data, this->test);
  }
  inline Work_t Work::get_work (const size_t i)
  noexcept {
    return (i < this->task_list.size()) ? this->task_list [i] : nullptr;
  }
  inline Work_t Work::get_work (const std::vector<size_t> hier)
  noexcept {
    Work_t W = nullptr;
    const auto sz = hier.size();
    if (sz > 0) {
      W = this->get_work (hier[0]);
      if (sz > 1) {
	for (size_t i = 1; i < sz; i++) {
	  W = (W == nullptr) ? nullptr : W->get_work (hier[i]);
    } } }
    return W;
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
	int Werr=0;
        try { Werr = W->exit (err); }
	catch (std::exception& e) { Werr = 1; }
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