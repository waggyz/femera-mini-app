#ifndef FEMERA_DATA_BANK_IPP
#define FEMERA_DATA_BANK_IPP

namespace femera {
  inline
  data::Bank::Bank (const Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name      ="Femera data values";
    this->abrv      ="vals";
    this->task_type = task_cast (Plug_type::Bank);
    this->info_d    = 3;
  }
  inline
  void data::Bank::task_init (int*, char**) {
  }
  inline
  void data::Bank::task_exit () {
  }
}//end femera namespace

//end FEMERA_DATA_BANK_IPP
#endif
