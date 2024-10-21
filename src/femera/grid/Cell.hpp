#ifndef FEMERA_HAS_GRID_CELL_HPP
#define FEMERA_HAS_GRID_CELL_HPP

#include "../Grid.hpp"

namespace femera { namespace grid {
  class Cell;// Derive a CRTP concrete class from Grid.
  class Cell final: public Grid<Cell> { friend class Grid;// Interface to Grid class
  private:
//    void scan      (int* argc, char** argv);
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Cell (femera::Work::Core_ptrs_t) noexcept;
    Cell () =delete;//NOTE Use the constructor above.
  };
} }//end femera::grid:: namespace

#include "Cell.ipp"

//end FEMERA_HAS_GRID_CELL_HPP
#endif
