#ifndef FEMERA_HAS_SIMS_IPP
#define FEMERA_HAS_SIMS_IPP

namespace femera {
  inline
  task::Sims::Sims (const femera::Work::Core_ptrs_t W)
  noexcept : Task (W) {
    this->name      ="Femera simulation handler";
    this->abrv      ="sims";
    this->task_type = task_cast (Task_type::Sims);
    this->thrd_ownr = fmr::Local_int (::omp_get_thread_num ());
  }
  inline
  void task::Sims::task_exit () {
  }
}//end femera namespace
//end FEMERA_HAS_SIMS_IPP
#endif
