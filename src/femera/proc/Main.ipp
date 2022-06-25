#ifndef FEMERA_MAIN_IPP
#define FEMERA_MAIN_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif
namespace femera {
  inline
  void proc::Main::task_exit () {
    this->proc =nullptr;
  }
}//end femera:: namespace
#undef FMR_DEBUG
//end FEMERA_MAIN_IPP
#endif
