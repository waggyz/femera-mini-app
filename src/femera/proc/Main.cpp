#include "Main.hpp"
#include "Ftop.hpp"
#include "Fcpu.hpp"

#ifdef FMR_HAS_MPI
#include "Fmpi.hpp"
#endif
#ifdef FMR_HAS_OMP
#include "Fomp.hpp"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void proc::Main:: task_init (int*, char**) {
    const auto core = this->get_core();
    std::vector<fmr::Local_int> path ={};
    path.push_back (this->add_task (Proc<proc::Ftop>::new_task (core)));
#ifdef FMR_HAS_MPI
    path.push_back (get_task_raw(path)->add_task (Proc<proc::Fmpi>::new_task (core)));
#endif
#ifdef FMR_HAS_OMP
    path.push_back (get_task_raw(path)->add_task (Proc<proc::Fomp>::new_task (core)));
#endif
    this->get_task_raw(path)->add_task (Proc<proc::Fcpu>::new_task (core));
#ifdef FMR_DEBUG
    //for (const auto P : this->task_list) { P->init (argc, argv); }
    printf ("Main: task_init added %s\n", this->name.c_str());
    auto P2 = this->get_task_raw(0);
    while (P2!=nullptr) {
      printf ("Main: task_init added %s\n", P2->name.c_str());
      P2 = P2->get_task_raw(0);
    }
#endif
  }
}// end femera::namespace

#undef FMR_DEBUG
