#ifndef FEMERA_HAS_PHYS_ELAS_IPP
#define FEMERA_HAS_PHYS_ELAS_IPP

namespace femera {
  inline
  phys::Elas::Elas (const femera::Work::Core_ptrs_t core)
  noexcept : Phys (core) {
    this->name      ="Femera elastic physics";
    this->abrv      ="elas";
    this->task_type = task_cast (Task_type::Elas);
  }
  inline
  void phys::Elas::task_init (int*, char**) {
    this->set_init (true);
  }
  inline
  void phys::Elas::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_PHYS_ELAS_IPP
#endif
