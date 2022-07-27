#include "Cgns.hpp"
#include "../proc/Main.hpp"

#ifdef FMR_HAS_CGNS
//#include "hdf5.h"
#ifdef FMR_HAS_MPI
#include "pcgnslib.h" // cgp_*
#else
#include "cgnslib.h"  // cg_*
#endif
#endif

namespace femera {
  void data::Cgns::task_init (int*, char**) {
    this->version = std::to_string (CGNS_VERSION / 1000)
      + "." + std::to_string (CGNS_VERSION % 1000);
#ifdef FMR_HAS_MPI
    FMR_PRAGMA_OMP(omp master)
    if (this->proc != nullptr) {// Set CGNS comm to a copy of Fmpi::team_id
      const auto P = this->proc->get_task (Task_type::Fmpi);
      if (P != nullptr) {
        MPI_Comm c;
        const auto err = MPI_Comm_dup (MPI_Comm (P->get_team_id()), &c);
        if (err) {//TODO exception should remove Cgns from the data task_list
          FMR_THROW("Failed to copy MPI communicator for CGNS.");
        } else {
          this->team_id = fmr::Team_int (c);
          this->version+=" (parallel)";
        }
#if 0
        if(this->proc->log->detail > 1 ){
          this->proc->log->label_printf("CGNS mode",
            "Parallel access using %i MPI threads/team\n",//TODO?
            this->proc->task.first<Proc>( Task_type:: Pmpi )->get_proc_n() );
        }
#endif
    } }
#endif
  }
  void data::Cgns::task_exit () {
//TODO    this->close_all ();
//TODO?   FMR_PRAGMA_OMP(omp barrier)
#ifdef FMR_HAS_MPI
    FMR_PRAGMA_OMP(omp master)
      if (this->team_id != fmr::Team_int (MPI_COMM_WORLD)) {
        MPI_Comm c = MPI_Comm (this->team_id);
        const auto err = MPI_Comm_free (&c);
        if (err) {
          const std::string msg = "Failed to free CGNS MPI communicator "
            + std::to_string (this->team_id)+".";
          FMR_THROW(msg);
        } else {
          this->team_id = 0;
      } }
#endif
  }
  bool data::Cgns::does_file (const fmr::Data_name_t& fname) {int err=0, fmt=0;
    err= cg_is_cgns (fname.c_str(), &fmt);
    return err == 0;
  }
}//end femera:: namespace


