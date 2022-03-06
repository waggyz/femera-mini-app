#ifndef FEMERA_DATA_Gmsh_IPP
#define FEMERA_DATA_Gmsh_IPP

namespace femera {
  inline
  data::Gmsh::Gmsh (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="Gmsh";
    this->abrv ="Gmsh";
    this->task_type = task_cast (Plug_type::Gmsh);
    this->info_d = 3;
  }
  inline
  void data::Gmsh::task_init (int*, char**) {;
  }
  inline
  void data::Gmsh::task_exit () {}
}//end femera namespace

//end FEMERA_DATA_Gmsh_IPP
#endif
