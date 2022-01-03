#ifndef FEMERA_FOMP_IPP
#define FEMERA_FOMP_IPP

namespace femera {
  inline
  proc::Fomp::Fomp (femera::Work::Core_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="OpenMP";
  }
  inline
  proc::Fomp::Fomp () noexcept {
    this->name ="OpenMP";
    this->info_d = 3;
  }
  inline
  void proc::Fomp::task_init (int*, char**){}
  inline
  void proc::Fomp::task_exit () {
  }
}//end femera namespace

//end FEMERA_FOMP_IPP
#endif
