#ifndef FEMERA_FCPU_IPP
#define FEMERA_FCPU_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  inline
  proc::Fcpu::Fcpu (const femera::Work::Core_ptrs_t core)
  noexcept : Proc (core) {
    this->name      = std::string ( MAKESTR(FMR_CPUMODEL) );
    this->abrv      ="cpu";
    this->version   = "";//TODO
    this->task_type = task_cast (Task_type::Fcpu);
    this->info_d    = 3;
  }
  inline
  void proc::Fcpu::task_init (int*, char**) {
  }
  inline
  void proc::Fcpu::task_exit () {
  }
}//end femera namespace

#undef FMR_DEBUG

//end FEMERA_FCPU_IPP
#endif
