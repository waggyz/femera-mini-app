#ifndef FEMERA_FCPU_IPP
#define FEMERA_FCPU_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  inline
  proc::Fcpu::Fcpu (femera::Work::Core_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="CPU";
    this->info_d = 3;
  }
  inline
  void proc::Fcpu::task_init (int*, char**){}
  inline
  void proc::Fcpu::task_exit () {
  }
}//end femera namespace

#undef FMR_DEBUG

//end FEMERA_FCPU_IPP
#endif
