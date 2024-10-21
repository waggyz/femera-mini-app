#ifndef FEMERA_HAS_GRID_FEMS_HPP
#define FEMERA_HAS_GRID_FEMS_HPP

namespace femera {
  inline
  grid::Fems::Fems (const femera::Work::Core_ptrs_t core)
  noexcept : Grid (core) {
    this->name      ="Femera femstic gridics";
    this->abrv      ="fems";
    this->task_type = task_cast (Task_type::Fems);
  }
  inline
  void grid::Fems::task_init (int*, char**) {
    this->set_init (true);
  }
  inline
  void grid::Fems::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_GRID_FEMS_HPP
#endif
