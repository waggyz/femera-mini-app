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
    const auto this_ptrs = this->ptrs();
    std::vector<fmr::Local_int> path ={};
//    auto task_i = this->add_task (Proc<proc::Ftop>::new_task ());
//      (std::make_shared<proc::Ftop> (proc::Ftop(this_ptrs)));
    path.push_back (this->add_task (Proc<proc::Ftop>::new_task ()));
#ifdef FMR_HAS_MPI
    path.push_back (get_task(path)->add_task (Proc<proc::Fmpi>::new_task ()));
#endif
#ifdef FMR_HAS_OMP
    path.push_back (get_task(path)->add_task (Proc<proc::Fomp>::new_task ()));
#endif
    this->get_task(path)->add_task (Proc<proc::Fcpu>::new_task ());
#ifdef FMR_DEBUG
    //for (const auto P : this->task_list) { P->init (argc, argv); }
    printf ("%s\n", this->name.c_str());
    auto P2 = this->get_task(0);
    while (P2!=nullptr) {
      printf ("%s\n", P2->name.c_str());
      P2 = P2->get_task(0);
    }
#endif
  }
}// end femera::namespace

#undef FMR_DEBUG
