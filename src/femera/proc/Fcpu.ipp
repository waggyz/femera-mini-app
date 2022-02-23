#ifndef FEMERA_FCPU_IPP
#define FEMERA_FCPU_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  inline
  proc::Fcpu::Fcpu (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name = std::string ( MAKESTR(FMR_CPUMODEL) );
    this->info_d = 3;
    this->abrv ="cpu";
    this->version = "";//FIXME
    this->task_type = task_cast (Plug_type::Fcpu);
  }
  inline
  void proc::Fcpu::task_init (int*, char**) {
//FIXME remove?     proc->Proc::set_base_n ();
  }
  inline
  void proc::Fcpu::task_exit () {
  }
#if 0
  inline
  fmr::Local_int proc::Fcpu::task_proc_ix () {
    return this->proc_ix;
  }
#endif
}//end femera namespace

#undef FMR_DEBUG

//end FEMERA_FCPU_IPP
#endif
