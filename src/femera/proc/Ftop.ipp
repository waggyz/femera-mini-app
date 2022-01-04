#ifndef FEMERA_FTOP_IPP
#define FEMERA_FTOP_IPP

namespace femera {
  inline
  proc::Ftop::Ftop (femera::Work::Core_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="top";
    this->info_d = 3;
  }
  inline
  void proc::Ftop::task_init (int*, char**){}
  inline
  void proc::Ftop::task_exit () {
  }
}//end femera namespace

//end FEMERA_FTOP_IPP
#endif
