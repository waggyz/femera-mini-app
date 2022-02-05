#include "Nvid.hpp"
#include "nvid.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
#ifdef FMR_HAS_NVIDIA
  proc::Nvid::Nvid (femera::Work::Core_ptrs core) noexcept {
    std::tie (this->proc, this->data, this->test) = core;
    this->name ="NVIDIA";
    this->abrv ="gpu";
    this->proc_n  = fmr::Local_int (999);
  }
  void proc::Nvid::task_init (int*, char**) {
  }
  bool proc::Nvid::is_in_parallel () {
    return false;
  }
#endif
}//end femera:: namespace

#undef FMR_DEBUG
