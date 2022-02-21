#include "Nvid.hpp"
#include "nvid.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
#ifdef FMR_HAS_NVIDIA
  proc::Nvid::Nvid (const femera::Work::Core_ptrs core) noexcept {
    std::tie (this->proc, this->data, this->test) = core;
    this->name ="NVIDIA";
    this->abrv ="gpu";
    this->task_type = task_cast (Plug_type::Nvid);
  }
  void proc::Nvid::task_init (int*, char**) {
#if 1
    this->team_n = proc::nvid::get_node_card_n ();
    this->name = proc::nvid::get_card_name (0);//FIXME loop
#endif
  }
  bool proc::Nvid::is_in_parallel () {
    return false;
  }
#endif
}//end femera:: namespace

#undef FMR_DEBUG
