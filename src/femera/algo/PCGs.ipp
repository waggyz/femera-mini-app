#ifndef FEMERA_HAS_ALGO_SPCG_IPP
#define FEMERA_HAS_ALGO_SPCG_IPP

namespace femera {
  inline
  algo::PCGs::PCGs (const femera::Work::Core_ptrs_t core)
  noexcept : Algo (core) {
    this->name      ="Femera elastic algoics";
    this->abrv      ="elas";
    this->task_type = task_cast (Task_type::PCGs);
  }
  inline
  void algo::PCGs::task_init (int*, char**) {
    this->set_init (true);
  }
  inline
  void algo::PCGs::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_ALGO_SPCG_IPP
#endif