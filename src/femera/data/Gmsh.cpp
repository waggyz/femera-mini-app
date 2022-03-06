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
      const bool read_gmsh_config = false;
      ::gmsh::initialize (0, nullptr, read_gmsh_config);// init without args.
      this->did_gmsh_init = true;
      ::gmsh::option::setNumber ("General.Verbosity", Gmsh::Optval(0));
      // gmsh::initialize (..) seeems to set omp_num_threads to 1,
      // so reset omp_num_threads.
#ifdef _OPENMP
#if 0
      //TODO liblock instead of omp master?
      auto P = this->proc->task.first<Proc> (Plug_type::Pomp);
      if (P) {
        const auto n = P->get_proc_n();
        this-> max_open_n = n;//TODO for 1 sim/omp (XS)?
        //this-> max_open_n = this->data->get_redo_n () / n + 1;//TODO
        gmsh::option::setNumber ("General.NumThreads",Gmsh::Optval(n));
        if (this->proc->log->detail > this->verblevel){
          this->proc->log->label_printf ("Gmsh uses",
            "%s %i OpenMP thread%s each.\n",
            (n==1)?"only":"up to", n, (n==1)?"":"s");
      } }
#else
      ::gmsh::option::setNumber ("General.NumThreads", Gmsh::Optval(2));//TODO
#endif
#endif
  } }
  void data::Gmsh::task_exit () {
    FMR_PRAGMA_OMP(omp master) {
      if (this->did_gmsh_init) {
        ::gmsh::finalize ();
  } } }
}//end femera:: namespace


