#include "Sims.hpp"
#include "../core.h"
#include "Runs.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void task::Sims::task_init (int*, char**) {
    fmr::Local_int o = 1;
#ifdef FMR_RUNS_LOCAL
    o = this->proc->get_race_n ();
#endif
    FMR_PRAGMA_OMP(omp parallel for schedule(static) ordered num_threads(o))
    for (fmr::Local_int i=0; i<o; ++i) {// Make & add thread-local Sims...
      FMR_PRAGMA_OMP(omp ordered) {     // ...in order.
        const auto R = Data<task::Runs>::new_task (this->get_core());
#ifdef FMR_RUNS_LOCAL
        R->set_name ("Femera simulations running on process "
          + std::to_string (this->proc->get_proc_id ()));
#endif
#ifdef FMR_DEBUG
        const auto m = this->proc->get_proc_ix (Task_type::Fmpi);
        printf ("%s (MPI: %u)\n", R->get_name ().c_str(), m);
#endif
        this->add_task (std::move (R));
  } }
  this->set_init (true);
  }//
  //
}//end femera namespace
//
#undef FMR_DEBUG
