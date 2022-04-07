#ifndef FEMERA_DATA_VALS_IPP
#define FEMERA_DATA_VALS_IPP

namespace femera {
  inline
  data::Vals::Vals (const Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name      ="Femera data values";
    this->abrv      ="vals";
    this->task_type = task_cast (Plug_type::Vals);
    this->info_d    = 3;
  }
  inline
  void data::Vals::task_exit () {
  }
}//end femera namespace

//end FEMERA_DATA_VALS_IPP
#endif
