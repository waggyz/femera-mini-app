#ifndef FEMERA_FMPI_IPP
#define FEMERA_FMPI_IPP

# include "mpi.h"

namespace femera {
  inline
  proc::Fmpi::Fmpi (femera::Work::Make_work_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
  }
  inline
  proc::Fmpi::Fmpi () noexcept {
    this->name ="MPI";
    this->info_d = 3;
  }
  inline
  void proc::Fmpi::task_exit () {
  }
}//end femera namespace

//end FEMERA_FMPI_IPP
#endif
