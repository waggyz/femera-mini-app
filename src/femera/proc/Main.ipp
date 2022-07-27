#ifndef FEMERA_MAIN_IPP
#define FEMERA_MAIN_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif
namespace femera {
# if 1
  inline
  fmr::Local_int proc::Main::get_race_n () {
    if (this->race_n > 0) { return this->race_n; }
    const auto O = this->get_task (Task_type::Fomp);
    if (O != nullptr) {
      this->race_n = O->get_proc_n ();
      return this->race_n;
    }
    return 0;
  }
#   endif
  inline
  void proc::Main::task_exit () {
    this->proc =nullptr;
  }
}//end femera:: namespace
#undef FMR_DEBUG
//end FEMERA_MAIN_IPP
#endif
