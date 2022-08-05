#ifndef FEMERA_ROOT_IPP
#define FEMERA_ROOT_IPP

namespace femera {
  inline
  proc::Root::Root (const femera::Work::Core_ptrs_t W)
  noexcept : Proc (W) {
    this->name      ="Femera root process";
    this->abrv      ="root";
    this->task_type = task_cast (Task_type::Root);
  }
  inline
  void proc::Root::task_init (int*, char**) {
# if 0
    this->data->send (fmr::log, "proc", "root", "NOTE",//TODO in Root.cpp
      "process handler initialization reported in reverse order");
#   endif
    this->set_init (true);
  }
  inline
  void proc::Root::task_exit () {
  }
}//end femera namespace

//end FEMERA_ROOT_IPP
#endif
