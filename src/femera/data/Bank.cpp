#include "Bank.hpp"
#include "../proc/Main.hpp"

#define FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void data::Bank::task_init (int*, char**) {
#ifdef FMR_VALS_LOCAL
#if 0
    this->vals.clear ();// should already exist on main thread, if at all
    this->vals.shrink_to_fit ();
#endif
    const auto n = this->proc->get_race_n ();
    this->vals.reserve (n);// allocate on main OpenMP thread
    FMR_PRAGMA_OMP(omp parallel for schedule(static) ordered num_threads(n))
    for (fmr::Local_int i=0; i<n; ++i) {// Make & add thread-local data::Bank
      FMR_PRAGMA_OMP(omp ordered) {     // in order.
        this->vals.push_back (data::Vals ());
    } }
    this->name += " with "+std::to_string (n)+" thread-local vals each";
# endif
    this->set_init (true);
  }
  data::Vals* data::Bank::vals_ptr () {
    return & vals [this->proc->get_race_ix ()];
  }
}//end femera:: namespace

#undef FMR_DEBUG