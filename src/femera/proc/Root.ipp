#ifndef FEMERA_ROOT_IPP
#define FEMERA_ROOT_IPP

namespace femera {
  inline
  proc::Root::Root (const femera::Work::Core_ptrs_t W) noexcept {
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
}//end femera namespace

//end FEMERA_ROOT_IPP
#endif
