#ifndef FEMERA_ROOT_IPP
#define FEMERA_ROOT_IPP

namespace femera {
  inline
  proc::Root::Root (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="root process";
    this->abrv ="root";
    this->task_type = task_cast (Plug_type::Root);
    this->info_d = 3;
  }
  inline
  void proc::Root::task_init (int*, char**){
  }
  inline
  void proc::Root::task_exit () {
  }
#if 0
  inline
  fmr::Local_int proc::Root::task_proc_ix () {
    return this->proc_ix;
  }
#endif
}//end femera namespace

//end FEMERA_ROOT_IPP
#endif
