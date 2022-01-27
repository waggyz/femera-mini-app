#ifndef FEMERA_FOMP_IPP
#define FEMERA_FOMP_IPP

namespace femera {
# if 0
  inline
  proc::Fomp::Fomp (femera::Work::Core_t core) noexcept {
    std::tie (this->proc, this->data, this->test) = core;
    this->name ="OpenMP";
  }
#   endif
  inline
  void proc::Fomp::task_init (int*, char**){
    this->proc_n = 2;//set from args or calculate from node_n*node_core_n /...
  }
  inline
  void proc::Fomp::task_exit () {}
}//end femera namespace

//end FEMERA_FOMP_IPP
#endif
