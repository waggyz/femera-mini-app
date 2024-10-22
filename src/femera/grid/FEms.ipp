#ifndef FEMERA_HAS_GRID_FEMS_HPP
#define FEMERA_HAS_GRID_FEMS_HPP

namespace femera {
  inline
  grid::FEms::FEms (const femera::Work::Core_ptrs_t core)
  noexcept : Grid (core) {
    this->name      ="Femera femstic gridics";
    this->abrv      ="fems";
    this->task_type = task_cast (Task_type::FEms);
  }
  inline
  void grid::FEms::task_init (int*, char**) {
    this->set_init (true);
  }
  inline
  void grid::FEms::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_GRID_FEMS_HPP
#endif
