#ifndef FEMERA_FMPI_IPP
#define FEMERA_FMPI_IPP

namespace femera {
  inline
  proc::Fmpi::Fmpi (femera::Work::Core_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="MPI";
    this->info_d = 3;
  }
#if 0
  inline
  proc::Fmpi::Fmpi () noexcept {
    this->name ="MPI";
    this->info_d = 3;
  }
#endif
  inline
  void proc::Fmpi::task_init (int*, char**){}
  inline
  void proc::Fmpi::task_exit () {
  }
}//end femera namespace

//end FEMERA_FMPI_IPP
#endif
