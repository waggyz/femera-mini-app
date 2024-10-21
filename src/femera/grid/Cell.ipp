#ifndef FEMERA_HAS_GRID_CELL_IPP
#define FEMERA_HAS_GRID_CELL_IPP

namespace femera {
  inline
  grid::Cell::Cell (const femera::Work::Core_ptrs_t W)
  noexcept : Grid (W) {
    this->name      ="Femera grid types";
    this->abrv      ="cell";
    this->task_type = task_cast (Task_type::Cell);
  }
  inline
  void grid::Cell::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_GRID_CELL_IPP
#endif
