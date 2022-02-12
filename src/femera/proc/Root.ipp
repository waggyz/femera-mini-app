#ifndef FEMERA_FTOP_IPP
#define FEMERA_FTOP_IPP

namespace femera {
  inline
  proc::Root::Root (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="root process";
    this->info_d = 3;
    this->abrv ="top";
  }
  inline
  void proc::Root::task_init (int*, char**){
  }
  inline
  void proc::Root::task_exit () {
  }
  inline
  fmr::Local_int proc::Root::task_proc_ix () {
    return this->proc_ix;
  }
}//end femera namespace

//end FEMERA_FTOP_IPP
#endif
