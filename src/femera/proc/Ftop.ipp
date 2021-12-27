#ifndef FEMERA_FTOP_IPP
#define FEMERA_FTOP_IPP

namespace femera {
  inline
  proc::Ftop::Ftop (femera::Work::Make_work_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="top";
    this->info_d = 3;
  }
  inline
  proc::Ftop::Ftop () noexcept {
    this->name ="top";
    this->info_d = 3;
  }
  inline
  void proc::Ftop::task_exit () {
  }
}//end femera namespace

//end FEMERA_FTOP_IPP
#endif
