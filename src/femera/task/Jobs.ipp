#ifndef FEMERA_HAS_JOBS_IPP
#define FEMERA_HAS_JOBS_IPP

namespace femera {
  inline
  task::Jobs::Jobs (femera::Work::Core_ptrs_t W)
  noexcept : Task (W) {
    this->name      ="Femera simulation job handler";
    this->abrv      ="jobs";
    this->task_type = task_cast (Plug_type::Jobs);
    this->info_d    = 2;
  }
  inline
  task::Jobs::Jobs () noexcept {
    this->name      ="simulation job handler";
    this->abrv      ="jobs";
    this->task_type = task_cast (Plug_type::Jobs);
    this->info_d    = 2;
  }
  inline
  void task::Jobs::task_exit () {
#ifdef FMR_DEBUG
    FMR_THROW("Jobs::task_exit() test throw.");
#endif
  }
}//end femera namespace

//end FEMERA_HAS_JOBS_IPP
#endif
