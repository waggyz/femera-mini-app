#ifdef FMR_HAS_NVIDIA
#ifndef FEMERA_NVID_IPP
#define FEMERA_NVID_IPP

namespace femera {
  inline
  void proc::Nvid::task_exit () {}
  inline
  fmr::Local_int proc::Nvid::task_proc_ix () {
    return this->proc_ix;
  }
}//end femera namespace

//end FEMERA_NVID_IPP
#endif
#endif
