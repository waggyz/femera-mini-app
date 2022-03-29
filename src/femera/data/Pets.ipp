#ifndef FEMERA_DATA_PETSC_IPP
#define FEMERA_DATA_PETSC_IPP

namespace femera {
  inline
  data::Pets::Pets (const femera::Work::Core_ptrs_t W)
  noexcept : Data (W) {
//    std::tie (this->proc, this->data, this->test) = W;
    this->name ="PETSc";
    this->abrv ="petc";
    this->task_type = task_cast (Plug_type::Petsc);
    this->info_d = 3;
  }
}//end femera namespace

//end FEMERA_DATA_PETSC_IPP
#endif
