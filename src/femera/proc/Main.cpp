#include "Main.hpp"
#include "Root.hpp"
#include "Node.hpp"
#include "Fcpu.hpp"

#ifdef FMR_HAS_MPI
#include "Fmpi.hpp"
#endif
#ifdef _OPENMP
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
  void proc::Main:: task_init (int*, char**) {
    const auto core = this->get_core ();
    Work::Task_path_t path ={};
#if 1
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
#ifdef _OPENMP
#ifdef FMR_OMP_LOCAL
    //TODO Add thread-local Fomp instances?
    const fmr::Local_int n = 2;//TODO calc or get from command arg
    FMR_PRAGMA_OMP(omp parallel for schedule(static) ordered num_threads(n))
    for (fmr::Local_int i=0; i<n; i++) {
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
#else // ifndef _OPENMP
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
  fmr::Local_int proc::Main::task_proc_ix () {
#ifdef FMR_DEBUG
    printf ("%s Main::task_proc_ix %u\n", get_abrv ().c_str(), proc_ix);
#endif
    switch (fmr::Enum_int (this->task_type)) {
      case (fmr::Enum_int (Plug_type::Fomp)):{
        return cast_via_work<proc::Fomp> (this)->task_proc_ix ();
    } }
    return this->proc_ix;
  }
#if 0
  fmr::Local_int proc::Main::auto_proc_n () {//TODO remove?
    fmr::Local_int all_n = 1;
    auto P = this->proc;
    if (P != nullptr) {
      all_n = P->all_proc_n ();//TODO wrong?
      fmr::Local_int all_core_n = 1;
      while (! P->task_list.empty ()) {
        P = P->get_task (0);
        switch (fmr::Enum_int (P->task_type)) {
          case (fmr::Enum_int(Plug_type::Node)): {
            const auto N = cast_via_work<proc::Node> (P);
            all_core_n = N->node_n * N->get_core_n ();// correct
            break;
          }
          case (fmr::Enum_int(Plug_type::Fomp)): {
#ifdef FMR_DEBUG
            printf ("%s Main::auto_proc_n set %s proc_n = %u / %u = %u\n",
              this->get_abrv ().c_str(), P->get_abrv ().c_str(),
                all_core_n, all_n, all_core_n / all_n);
#endif
            if (all_core_n > all_n && all_n >0 && all_core_n >0) {
              //cast_via_work<proc::Fomp>(P)->set_proc_n ();
              P->proc_n = all_core_n / all_n;
            }
            all_n = this->proc->all_proc_n ();
            break;
    } } } }
    return all_n;
  }
#endif
}// end femera::namespace

#undef FMR_DEBUG
