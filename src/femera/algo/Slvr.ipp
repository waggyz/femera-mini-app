#ifndef FEMERA_HAS_ALGO_SLVR_IPP
#define FEMERA_HAS_ALGO_SLVR_IPP

namespace femera {
  inline
  algo::Slvr::Slvr (const femera::Work::Core_ptrs_t W)
  noexcept : Algo (W) {
    this->name      ="Femera solvers";
    this->abrv      ="slvr";
    this->task_type = task_cast (Task_type::Slvr);
  }
  inline
  void algo::Slvr::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_ALGO_SLVR_IPP
#endif
