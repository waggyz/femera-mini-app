#include "Gmsh.hpp"

#ifdef FMR_HAS_GMSH
#include "gmsh.h"
#endif

namespace femera {
  data::Gmsh::Gmsh (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name      ="Gmsh";
    this->abrv      ="gmsh";
    this->version   = GMSH_API_VERSION ;
    this->task_type = task_cast (Plug_type::Gmsh);
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
      const auto P = this->proc->get_task (Plug_type::Fomp);
      if (P != nullptr) {
        ::gmsh::option::setNumber ("General.NumThreads",
          Gmsh::Number (P->get_proc_n ()));
      }
      if (true) {//TODO detail
        Gmsh::Number v = 0;
        ::gmsh::option::getNumber ("General.NumThreads", v);
        const auto n = fmr::Local_int (v);
        this->data->name_line (this->data->fmrlog, "data gmsh uses",
          "%4u OpenMP thread%s each (maximum)", n, (n==1)?"":"s");
  } } }
  void data::Gmsh::task_exit () {
    FMR_PRAGMA_OMP(omp master) {
      if (this->did_gmsh_init) {
        ::gmsh::finalize ();
  } } }
}//end femera:: namespace


