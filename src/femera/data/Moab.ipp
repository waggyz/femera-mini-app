#ifndef FEMERA_DATA_Moab_IPP
#define FEMERA_DATA_Moab_IPP

namespace femera {
  inline
  data::Moab::Moab (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="MOAB";
    this->abrv ="Moab";
    this->task_type = task_cast (Plug_type::Moab);
    this->info_d = 3;
  }
  inline
  void data::Moab::task_init (int*, char**) {
  }
  inline
  void data::Moab::task_exit () {}
}//end femera namespace

//end FEMERA_DATA_Moab_IPP
#endif