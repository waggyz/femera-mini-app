#include "../core.h"
#include "Gmsh.hpp"

#ifdef FMR_HAS_GMSH
#include "gmsh.h"
#endif

namespace femera {
  data::Gmsh::Gmsh (const femera::Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name      ="Gmsh";
    this->abrv      ="gmsh";
    this->version   = GMSH_API_VERSION ;
#ifdef FMR_HAS_GMSH_GCC48_PATCH
    this->version  +=" (patched for GCC 4.8)" ;
#endif
    this->task_type = task_cast (Task_type::Gmsh);
    this->info_d    = 3;
  }
  void data::Gmsh::task_init (int*, char**) {
    FMR_PRAGMA_OMP(omp master) {
      const bool do_read_gmsh_config = false;
      ::gmsh::initialize (0, nullptr, do_read_gmsh_config);// init without args.
      this->did_gmsh_init = true;
      ::gmsh::option::setNumber ("General.Verbosity", Gmsh::Number(0));
      // gmsh::initialize (..) seeems to set omp_num_threads to 1 and
      // setting General.NumThreads also sets omp_num_threads
      const auto omp_n = this->proc->get_proc_n (Task_type::Fomp);
      if (omp_n > 0) {
        ::gmsh::option::setNumber ("General.NumThreads", Gmsh::Number (omp_n));
      }
      Gmsh::Number gmsh_omp_n = 0;
      ::gmsh::option::getNumber ("General.NumThreads", gmsh_omp_n);
      if (fmr::Local_int (gmsh_omp_n) == omp_n) {
        if (true) {//TODO detail level?
          const auto n = fmr::Local_int (gmsh_omp_n);
          this->data->send (fmr::log, "data", "gmsh", "uses",
            "%4u    /%4u OpenMP thread%s each (maximum)",
            n, omp_n, (omp_n==1)?"":"s");
        } }
      else {
        this->data->send (fmr::log, "data", "gmsh", "NOTE",
        "OpenMP threads (%g) does not match Femera (%u).",
        gmsh_omp_n, omp_n);
  } } }
  void data::Gmsh::task_exit () {
    FMR_PRAGMA_OMP(omp master) {
      if (this->did_gmsh_init) { ::gmsh::finalize ();
  } } }
}//end femera:: namespace


