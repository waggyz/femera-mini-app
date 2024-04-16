#include "../core.h"
#include "Cgns.hpp"

#ifdef FMR_HAS_CGNS
#ifdef FMR_HAS_HDF5
#include "hdf5.h"
#include "H5public.h" // defines H5_VERS_INFO
#endif
#ifdef FMR_HAS_MPI
#include "pcgnslib.h" // cgp_* and cg_*
#else
#include "cgnslib.h"  // cg_*
#endif
#endif

namespace femera {
  void data::Cgns::task_init (int*, char**) {
#if 0
    this->version
      = std::to_string (CGNS_VERSION / 1000) + "."
      + std::to_string (CGNS_VERSION % 1000);
#else
    this->version = MAKESTR(CGNS_DOTVERS);
#endif
#ifdef H5_VERS_INFO
    this->hdf5_vers = H5_VERS_INFO;// is already double-quoted
    this->data->send (fmr::info,
      get_base_abrv ().c_str(), get_abrv ().c_str(), "uses", H5_VERS_INFO);
#endif
#ifdef FMR_HAS_MPI
//NO_FMR_PRAGMA_OMP(omp MAIN)//NOTE OpenMP does not play nice with exceptions.
    if (this->proc != nullptr) {// Set CGNS comm to a copy of Fmpi::team_id
      const auto P = this->proc->get_task (Task_type::Fmpi);
      if (P != nullptr) {
        if (P->get_team_id() != 0) {
          MPI_Comm c = MPI_COMM_NULL;//nullptr for OpenMPI, 0 for MPICH;
          const auto err = MPI_Comm_dup (MPI_Comm (P->get_team_id()), &c);
          if (err || (c == MPI_COMM_NULL)) {//nullptr)) {
            printf ("%4s %4s %4s %s\n",
              get_base_abrv ().c_str(), get_abrv ().c_str(), "WARN",
              "failed to copy MPI communicator for CGNS");
            this->set_init (false);// parent removes uninitialized tasks.
            return;
          }
          this->team_id = fmr::Team_int (c);
          this->version+=" (parallel)";
    } } }
#endif
    this->set_init (true);
  }
  void data::Cgns::task_exit () {
//TODO    this->close_all ();
#ifdef FMR_HAS_MPI
//NO_FMR_PRAGMA_OMP(omp MAIN)//NOTE OpenMP does not play nice with exceptions.
    if (this->team_id != fmr::Team_int (MPI_COMM_WORLD)) {
      this->set_init (false);
      MPI_Comm c = MPI_Comm (this->team_id);
      if (c != MPI_COMM_NULL) {//nullptr) {
        const auto err = MPI_Comm_free (&c);
        if (err) {
          const std::string msg = "Failed to free CGNS MPI communicator "
            + std::to_string (this->team_id)+".";
          FMR_THROW(msg);
        } else {
         this->team_id = 0;
    } } }
#endif
  }
  bool data::Cgns::does_file (const fmr::Data_name_t& fname) {int err=0, fmt=0;
    err = cg_is_cgns (fname.c_str(), &fmt);
    return err == 0;
  }
}//end femera:: namespace