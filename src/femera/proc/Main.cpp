#include "Main.hpp"
#include "Root.hpp"
#include "Node.hpp"
#include "Fcpu.hpp"

#ifdef FMR_HAS_MPI
#include "Fmpi.hpp"
#endif
#ifdef FMR_HAS_OPENMP
#include "Fomp.hpp"
#endif
#ifdef FMR_HAS_NVIDIA
#include "Nvid.hpp"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  proc::Main::Main (const femera::Work::Core_ptrs_t W)
  noexcept : Proc (W) {
    this->name      ="Femera processing handler";
    this->abrv      ="main";
    this->task_type = task_cast (Task_type::Main);
    this->info_d    = 2;
  }
  proc::Main::Main () noexcept {
    this->name      ="Femera processing handler";
    this->abrv      ="main";
    this->task_type = task_cast (Task_type::Main);
    this->info_d    = 2;
  }
  void proc::Main:: task_init (int*, char**) {
    const auto core = this->get_core ();
    Work::Task_path_t path ={};
#ifdef FMR_USE_PROC_ROOT
    //TODO Is proc::Root needed?
    path.push_back (add_task (std::move (Proc<proc::Root>::new_task (core))));
    path.push_back (get_task (path)->add_task
      (std::move(Proc<proc::Node>::new_task (core))));
#else
    path.push_back (add_task (std::move (Proc<proc::Node>::new_task (core))));
#endif
#ifdef FMR_HAS_NVIDIA
    auto gpu_parent_path = path;
#endif
#ifdef FMR_HAS_MPI
    path.push_back (get_task (path)->add_task
      (std::move(Proc<proc::Fmpi>::new_task (core))));
#ifdef FMR_HAS_NVIDIA
    gpu_parent_path = path;
#endif
#endif
#ifdef FMR_HAS_OPENMP
#ifdef FMR_OMP_LOCAL
    //TODO Add thread-local Fomp instances?
    const fmr::Local_int n = 2;//TODO calc or get from command arg
    FMR_PRAGMA_OMP(omp parallel for schedule(static) ordered num_threads(n))
    for (fmr::Local_int i=0; i<n; ++i) {
      FMR_PRAGMA_OMP(omp ordered)
      path.push_back (get_task (path)->add_task
        (std::move(Proc <proc::Fomp>::new_task (core))));
      this->get_task (path)->add_task
        (std::move(Proc <proc::Fcpu>::new_task (core)));
      path.pop_back ();
    }
#else // ifndef FMR_OMP_LOCAL
    path.push_back (get_task (path)->add_task
      (std::move(Proc <proc::Fomp>::new_task (core))));
    this->get_task (path)->add_task
      (std::move(Proc <proc::Fcpu>::new_task (core)));
#endif // ifdef FMR_OMP_LOCAL
#else // ifndef FMR_HAS_OPENMP
    this->get_task (path)->add_task
      (std::move(Proc <proc::Fcpu>::new_task (core)));
#endif
#ifdef FMR_HAS_NVIDIA
    this->get_task (gpu_parent_path)->add_task
      (std::move(Proc <proc::Nvid>::new_task (core)));
#endif
#ifdef FMR_DEBUG
    //for (const auto P : this->task_list) { P->init (argc, argv); }
    printf ("Main: task_init added %s\n", this->get_name ().c_str());
    auto P2 = this->get_task (0);
    while (P2 != nullptr) {
      printf ("Main: task_init added %s\n", P2->get_name ().c_str());
      P2 = P2->get_task (0);
    }
#endif
  }
  fmr::Local_int proc::Main::task_proc_ix ()
  noexcept {
#ifdef FMR_DEBUG
    printf ("%s Main::task_proc_ix %u\n", get_abrv ().c_str(), proc_ix);
#endif
    switch (fmr::Enum_int (this->task_type)) {
      case (fmr::Enum_int (Task_type::Fomp)):{
        return Work::cast_via_work<proc::Fomp> (this)->task_proc_ix ();
    } }
    return this->proc_ix;
  }
}// end femera::namespace

#undef FMR_DEBUG
