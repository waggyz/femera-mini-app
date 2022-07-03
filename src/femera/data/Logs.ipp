#ifndef FEMERA_DATA_LOGS_IPP
#define FEMERA_DATA_LOGS_IPP

namespace femera {
  inline
  data::Logs::Logs (const femera::Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name      ="Femera logger";
    this->abrv      ="logs";
    this->task_type = task_cast (Task_type::Logs);
    this->info_d    = 3;
  }
  inline
  fmr::Dim_int data::Logs::set_verb (const fmr::Dim_int v)
  noexcept {
    if (v > FMR_VERBMAX) {//TODO print warning
      this->verb_d = FMR_VERBMAX;
    } else {
      this->verb_d = v;
    }
    return this->verb_d;
  }
  inline
  bool data::Logs::do_log (const fmr::Dim_int v)
  noexcept {
    return v >= this->verb_d;
  }
}//end femera namespace

//end FEMERA_DATA_LOGS_IPP
#endif
