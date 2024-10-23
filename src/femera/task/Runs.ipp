#ifndef FEMERA_HAS_RUNS_IPP
#define FEMERA_HAS_RUNS_IPP

namespace femera {
  inline
  task::Runs::Runs (const femera::Work::Core_ptrs_t W)
  noexcept : Task (W) {
    this->name      ="Femera simulation runner";
    this->abrv      ="runs";
    this->task_type = task_cast (Task_type::Runs);
#ifdef FMR_HAS_OPENMP
    this->name += " " + std::to_string (::omp_get_thread_num ());
    this->thrd_ownr = fmr::Local_int (::omp_get_thread_num ());
#endif
  }
  inline
  void task::Runs::task_init (int*, char**) {
    this->set_init (true);
  }
  inline
  void task::Runs::task_exit () {
  }
}//end femera namespace
//end FEMERA_HAS_RUNS_IPP
#endif
