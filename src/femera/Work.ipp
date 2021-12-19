#ifndef FMR_HAS_WORK_IPP
#define FMR_HAS_WORK_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
Work::Work (Work* W)
  noexcept : proc (W->proc), file (W->file), test (W->test) {
}
Work_t Work::get_work (size_t i)
noexcept {
  return (i < this->task_list.size()) ? this->task_list[i] : nullptr;
}
void Work::add_task (Work_t W)
noexcept {
  this->task_list.push_back(W);
}
size_t Work::get_task_n ()
noexcept {
  return this->task_list.size();
}
}// end femera:: namespace

#undef FMR_DEBUG
//end FMR_HAS_WORK_IPP
#endif