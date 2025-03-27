#ifndef FEMERA_HAS_MPI_HPP
#define FEMERA_HAS_MPI_HPP

#include "../Proc.hpp"

#ifdef FMR_HAS_MPI

namespace femera { namespace proc {
  class Fmpi;// Derive a CRTP concrete class from Proc.
  class Fmpi final: public Proc<Fmpi> { friend class Proc;
  private:
  //fmr::Team_int     comm = 0; default: MPI_COMM_WORLD (inherited from Proc)
    int     fmpi_required = 0;// default: MPI_THREAD_SERIALIZED
    int     fmpi_provided = 0;//TODO rqrd_mpi_thrd, prvd_mpi_thrd ?
    bool do_final_on_exit = true;//TODO should be false by default. See below.
   /* Technically, it is the user's responsibility to initialize and finalize
    * MPI, because it is the user starting MPI with mpirunmpiexec/mpirun.
    * However, I think it sould default to the following.
    *   * If MPI is already initialized when starting Femera, do not finalize on
    *     exit.
    *   * If MPI is not initialized when starting Femera, do finalize on exit.
    *   * User may override this behavior using a CLI argument or pre-init API
    *     call.
    */
  private:
    bool did_mpi_init () noexcept;
    fmr::Local_int task_proc_n  () noexcept;
#if 1
    fmr::Local_int task_proc_ix () noexcept;
#endif
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Fmpi (femera::Work::Core_ptrs_t) noexcept;
    Fmpi () = delete;//NOTE Use the constructor above.
  };
} }//end femera::proc:: namespace

#include "Fmpi.ipp"

#endif
//end FEMERA_HAS_MPI_HPP
#endif
