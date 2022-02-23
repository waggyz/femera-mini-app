#ifndef FEMERA_HAS_MPI_HPP
#define FEMERA_HAS_MPI_HPP

#include "../Proc.hpp"

#ifdef FMR_HAS_MPI

namespace femera { namespace proc {
  class Fmpi;// Derive a CRTP concrete class from Proc.
  class Fmpi : public Proc<Fmpi> {private: friend class Proc;
  private:
  //proc::Team_t     comm = 0; default: MPI_COMM_WORLD (in Proc)
    int     fmpi_required = 0;// default: MPI_THREAD_SERIALIZED
    int     fmpi_provided = 0;//TODO rqrd_mpi_thrd, prvd_mpi_thrd ?
    bool do_final_on_exit = true;
  private:
    bool did_mpi_init ();
    fmr::Local_int task_proc_n  ();
#if 1
    fmr::Local_int task_proc_ix ();
#endif
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Fmpi (femera::Work::Core_ptrs_t) noexcept;
    Fmpi () =delete;//NOTE Use the constructor above.
  };
} }//end femera::proc:: namespace

#include "Fmpi.ipp"

#endif
//end FEMERA_HAS_MPI_HPP
#endif
