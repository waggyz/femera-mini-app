#include "Pets.hpp"

#ifdef FMR_HAS_PETSC
//#include "hdf5.h"
//#include <petsc.h>
#include <petscsys.h>
#endif

namespace femera {
  namespace data {
#ifdef FMR_HAS_PETSC
    static char petsc_help[] = "Femera interface to PETSC.\n";
#endif
  }
  void data::Pets::task_init (int*, char**) {
#ifdef FMR_HAS_PETSC
//  PetscInitialize(argc,&argv,(char*)nullptr,data::petsc_help);//if (ierr) return ierr;
   PetscInitialize(nullptr,nullptr,(char*)nullptr,data::petsc_help);
   //
  char           ver[128];
  PetscInt       major,minor,subminor;
//  PetscInitialize(&argc,&argv,(char*)0,help);//if (ierr) return ierr;
  PetscGetVersion(ver,sizeof(ver));
  PetscGetVersionNumber(&major,&minor,&subminor,nullptr);
  this->set_name (std::string (ver));
  this->version = std::to_string (major) + "." + std::to_string (minor)
    + "." + std::to_string (subminor);
#endif
#if 0
    this->version = std::to_string (Pets_VERSION / 1000)
      + "." + std::to_string (Pets_VERSION % 1000);
#ifdef FMR_HAS_MPI
    FMR_PRAGMA_OMP(omp master)
    if (this->proc != nullptr) {// Set Pets comm to a copy of Fmpi::team_id
      const auto P = this->proc->get_task (Task_type::Fmpi);
      if (P != nullptr) {
        MPI_Comm c;
        const auto err = MPI_Comm_dup (MPI_Comm (P->get_team_id()), &c);
        if (err) {//TODO exception should remove Pets from the data task_list
          FMR_THROW("Failed to copy MPI communicator for Pets.");
        } else {
          this->team_id = proc::Team_t (c);
          this->version+=" (parallel)";
        }
#if 0
        if(this->proc->log->detail > 1 ){
          this->proc->log->label_printf("Pets mode",
            "Parallel access using %i MPI threads/team\n",//TODO?
            this->proc->task.first<Proc>( Task_type:: Pmpi )->get_proc_n() );
        }
#endif
    } }
#endif
#endif
  }
  void data::Pets::task_exit () {
#ifdef FMR_HAS_PETSC
    PetscFinalize();
#if 0
//TODO    this->close_all ();
//TODO?   FMR_PRAGMA_OMP(omp barrier)
#ifdef FMR_HAS_MPI
    FMR_PRAGMA_OMP(omp master)
      if (this->team_id != proc::Team_t (MPI_COMM_WORLD)) {
        MPI_Comm c = MPI_Comm (this->team_id);
        const auto err = MPI_Comm_free (&c);
        if (err) {
          const std::string msg = "Failed to free Pets MPI communicator "
            + std::to_string (this->team_id)+".";
          FMR_THROW(msg);
        } else {
          this->team_id = 0;
      } }
#endif
#endif
#endif
  }
}//end femera:: namespace


