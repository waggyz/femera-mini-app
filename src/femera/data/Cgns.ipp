#ifndef FEMERA_DATA_Cgns_IPP
#define FEMERA_DATA_Cgns_IPP

namespace femera {
  inline
  data::Cgns::Cgns (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="CGNS";
    this->abrv ="Cgns";
    this->task_type = task_cast (Plug_type::Cgns);
    this->info_d = 3;
  }
  inline
  void data::Cgns::task_init (int*, char**) {
  }
  inline
  void data::Cgns::task_exit () {}
}//end femera namespace

//end FEMERA_DATA_Cgns_IPP
#endif
