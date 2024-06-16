#include "Pets.hpp"

#ifdef FMR_HAS_PETSC
//#include "hdf5.h"
//#include <petsc.h>
#include <petscsys.h>

namespace femera {
  namespace data {
    static char petsc_help[] = "Femera interface to PETSc.\n";
  }
  void data::Pets::task_init (int*, char**) {
  //TODO Set PETSC_COMM_WORLD to a copy of MPI_COMM_WORLD or Femera's MPI communicator?.
  const auto ierr = PetscInitialize (nullptr, nullptr, (char*)nullptr, data::petsc_help);
  //FIXED it is not clear how to init PETSc on other than MPI_COMM_WORLD.
  //      Yes it is: https://petsc.org/release/manualpages/Sys/PETSC_COMM_WORLD/
  PetscInt major, minor, subminor;
  char     ver[128];
  //
  PetscGetVersionNumber (&major, &minor, &subminor, nullptr);
  PetscGetVersion (ver, sizeof(ver));
  this->set_name (std::string (ver));
  this->version
    =       std::to_string (major)
    + "." + std::to_string (minor)
    + "." + std::to_string (subminor);
  if (ierr <= 0) {
    this->set_init (true);
  }
#if 0
    this->version = std::to_string (Pets_VERSION / 1000)
      + "." + std::to_string (Pets_VERSION % 1000);
#ifdef FMR_HAS_MPI
//NO_FMR_PRAGMA_OMP(omp MAIN)//NOTE OpenMP does not play nice with exceptions.
    if (this->proc != nullptr) {// Set Pets comm to a copy of Fmpi::team_id
      const auto P = this->proc->get_task (Task_type::Fmpi);
      if (P != nullptr) {
        MPI_Comm c;
        const auto err = MPI_Comm_dup (MPI_Comm (P->get_team_id()), &c);
        if (err) {//TODO exception should remove Pets from the data task_list
          FMR_THROW("Failed to copy MPI communicator for Pets.");
        } else {
          this->team_id = fmr::Team_int (c);
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
    if (this->did_init ()) {
      PetscFinalize ();
      this->set_init (false);
    }
#if 0
//TODO    this->close_all ();
//TODO?   FMR_PRAGMA_OMP(omp barrier)
#ifdef FMR_HAS_MPI
//NO_FMR_PRAGMA_OMP(omp MAIN)//NOTE OpenMP does not play nice with exceptions.
      if (this->team_id != fmr::Team_int (MPI_COMM_WORLD)) {
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
  }
}//end femera:: namespace
#else
// ! FMR_HAS_PETSC
namespace femera {
  namespace data {}
  void data::Pets::task_init (int*, char**) {}
  void data::Pets::task_exit () {}
}//end femera:: namespace
#endif
// FMR_HAS_PETSC

