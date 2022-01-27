#ifndef FEMERA_FTOP_IPP
#define FEMERA_FTOP_IPP

namespace femera {
  inline
  proc::Root::Root (femera::Work::Core_t W) noexcept {
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
}//end femera namespace

//end FEMERA_FTOP_IPP
#endif
