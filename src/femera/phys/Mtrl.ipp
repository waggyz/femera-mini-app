#ifndef FEMERA_HAS_MTRL_IPP
#define FEMERA_HAS_MTRL_IPP

namespace femera {
  inline
  phys::Mtrl::Mtrl (const femera::Work::Core_ptrs_t W)
  noexcept : Phys (W) {
    this->name      ="Femera materials";
    this->abrv      ="mtrl";
    this->task_type = task_cast (Task_type::Mtrl);
  }
  inline
  void phys::Mtrl::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_MTRL_IPP
#endif
