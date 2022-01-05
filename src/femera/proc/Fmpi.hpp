#ifndef FEMERA_HAS_MPI_HPP
#define FEMERA_HAS_MPI_HPP

#include "../Proc.hpp"

#ifdef FMR_HAS_MPI

namespace femera { namespace proc {
  class Fmpi;// Derive a CRTP concrete class from Proc.
  class Fmpi : public Proc<Fmpi> {
  private:
    //proc::Team_t comm      = 0; default: MPI_COMM_WORLD (in Proc)
    int       mpi_required = 0;// default: MPI_THREAD_SERIALIZED
    int       mpi_provided = 0;//TODO rqrd_mpi_thrd, prvd_mpi_thrd ?
    fmr::Local_int proc_n  = 0;
    bool do_finalize_on_exit = true;
  public:
    Team_t get_team_id () noexcept;
    fmr::Local_int get_proc_n ();
    fmr::Local_int get_proc_i ();
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
