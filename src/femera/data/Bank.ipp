#ifndef FEMERA_DATA_BANK_IPP
#define FEMERA_DATA_BANK_IPP

namespace femera {
  inline
  data::Bank::Bank (const Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name      ="Femera data bank";
    this->abrv      ="bank";
    this->task_type = task_cast (Task_type::Bank);
    this->info_d    = 3;
  }
#ifdef FMR_BANK_LOCAL
  inline
  data::Vals* data::Bank::vals_ptr () {
    return & vals;
  }
#endif
#ifdef FMR_VALS_LOCAL
  inline
  data::Vals* data::Bank::vals_ptr (fmr::Local_int ix) {
    return & vals [ix];
  }
#endif
  inline
  void data::Bank::task_exit () {
  }
}//end femera namespace

//end FEMERA_DATA_BANK_IPP
#endif
