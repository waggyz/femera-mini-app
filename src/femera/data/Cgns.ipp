#ifndef FEMERA_DATA_CGNS_IPP
#define FEMERA_DATA_CGNS_IPP

namespace femera {
  inline
  data::Cgns::Cgns (const femera::Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name ="CGNS";
    this->abrv ="cgns";
    this->task_type = task_cast (Plug_type::Cgns);
    this->info_d = 3;
  }
}//end femera namespace

//end FEMERA_DATA_CGNS_IPP
#endif
