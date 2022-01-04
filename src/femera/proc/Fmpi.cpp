#include "Fmpi.hpp"

#ifdef FMR_HAS_MPI
#include "mpi.h"
namespace femera {
  //
  proc::Fmpi::Fmpi (femera::Work::Core_t W)
  noexcept
  : mpi_required {int (MPI_THREAD_SERIALIZED) } {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="MPI";
    this->info_d = 3;
    this->team_id = proc::Team_t (MPI_COMM_WORLD);
  }
  //
}// end femera:: namespace
//end if FMR_HAS_MPI
#endif