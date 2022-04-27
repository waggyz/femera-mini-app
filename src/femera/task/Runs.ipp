#ifndef FEMERA_HAS_RUNS_IPP
#define FEMERA_HAS_RUNS_IPP

namespace femera {
  inline
  task::Runs::Runs (femera::Work::Core_ptrs_t W)
  noexcept : Task (W) {
    this->name      ="Femera simulation runner";
    this->abrv      ="runs";
    this->task_type = task_cast (Plug_type::Runs);
    this->info_d    = 3;
  }
  inline
  void task::Runs::task_init (int*, char**) {
  }
  inline
  void task::Runs::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_RUNS_IPP
#endif