#ifndef FEMERA_MAIN_IPP
#define FEMERA_MAIN_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif
namespace femera {
  inline
  proc::Main::Main (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="processing";
    this->abrv ="main";
    this->task_type = task_cast (Plug_type::Main);
    this->info_d = 2;
  }
  inline
  proc::Main::Main () noexcept {
    this->name ="processing";
    this->abrv ="main";
    this->task_type = task_cast (Plug_type::Main);
    this->info_d = 2;
  }
  inline
  void proc::Main::task_exit () {
    this->proc =nullptr;
  }
}//end femera:: namespace
#undef FMR_DEBUG
//end FEMERA_MAIN_IPP
#endif
