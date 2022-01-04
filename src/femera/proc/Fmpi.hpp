#ifndef FEMERA_HAS_MPI_HPP
#define FEMERA_HAS_MPI_HPP

#include "../Proc.hpp"

#ifdef FMR_HAS_MPI

namespace femera { namespace proc {
  class Fmpi;// Derive a CRTP concrete class from Proc.
  class Fmpi : public Proc<Fmpi> {
  private:
//    proc::Team_t comm      = 0;//MPI_COMM_WORLD
    const int mpi_required = 0;//MPI_THREAD_SERIALIZED
    int       mpi_provided = 0;//TODO rqrd_mpi_thrd, prvd_mpi_thrd ?
  public:
    Team_t get_team_id () noexcept;
  public:
    Fmpi (femera::Work::Core_t) noexcept;
    Fmpi () =delete;//NOTE Use the constructor above.
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Fmpi.ipp"

#endif
//end FEMERA_HAS_MPI_HPP
#endif
