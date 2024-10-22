#ifndef FEMERA_HAS_PART_IPP
#define FEMERA_HAS_PART_IPP

namespace femera {
  inline
  task::Part::Part (const femera::Work::Core_ptrs_t W)
  noexcept : Task (W) {
    this->name      ="Femera partition";
    this->abrv      ="part";
    this->task_type = task_cast (Task_type::Part);
  }
  inline
  void task::Part::task_init (int*, char**) {
    this->set_init (true);
  }
  inline
  void task::Part::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_PART_IPP
#endif
