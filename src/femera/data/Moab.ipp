#ifndef FEMERA_DATA_Moab_IPP
#define FEMERA_DATA_Moab_IPP

namespace femera {
  inline
  data::Moab::Moab (const femera::Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name      ="MOAB";
    this->abrv      ="moab";
    this->task_type = task_cast (Task_type::Moab);
    this->info_d    = 3;
  }
  inline
  void data::Moab::task_init (int*, char**) {
    this->set_init (false);
  }
  inline
  void data::Moab::task_exit () {}
}//end femera namespace

//end FEMERA_DATA_Moab_IPP
#endif
