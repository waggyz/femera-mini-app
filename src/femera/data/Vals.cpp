#include "Vals.hpp"

namespace femera {
  void data::Vals::task_init (int*, char**) {
    fmr::Local_int n = 1;
#ifdef FMR_VALS_LOCAL
    if (this->proc != nullptr) {
      const auto P = this->proc->get_task (Plug_type::Fomp);
      if (P != nullptr) {
        n = P->get_proc_n ();
    } }
#endif
    FMR_PRAGMA_OMP(omp parallel for schedule(static) num_threads(n))
    for (fmr::Local_int i=0; i<n; i++) {// rename thread-local Vals
      auto V = this->get_task (Plug_type::Vals, i);
//      const auto P = this->proc->get_task (Plug_type::Fomp, i);
      if (V != nullptr) {// && P != nullptr) {
        V->set_name ("Femera data for process "
          + std::to_string (V->proc->get_proc_id ()));//FIXME no worky, n is 2
  } } }
}//end femera:: namespace


