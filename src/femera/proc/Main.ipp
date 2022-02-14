#ifndef FEMERA_MAIN_IPP
#define FEMERA_MAIN_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif
namespace femera {
  inline
  proc::Main::Main (const femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="processing";
    this->info_d = 2;
    this->abrv ="main";
  }
  inline
  proc::Main::Main () noexcept {
    this->name ="processing";
    this->info_d = 2;
    this->abrv ="main";
  }
  inline
  void proc::Main::task_exit () {
    this->proc =nullptr;
  }
#if 0
  inline
  fmr::Local_int proc::Main::task_proc_ix () {
#ifdef FMR_DEBUG
    printf ("%s Main::task_proc_ix %u\n", abrv.c_str(), proc_ix);
#endif
    return this->proc_ix;
  }
#endif
}//end femera:: namespace
#undef FMR_DEBUG
//end FEMERA_MAIN_IPP
#endif
