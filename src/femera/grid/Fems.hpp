#ifndef FEMERA_HAS_GRID_FEMS_HPP
#define FEMERA_HAS_GRID_FEMS_HPP

#include "../Grid.hpp"

namespace femera { namespace grid {
  class Fems;// Derive a CRTP concrete class from Grid.
  class Fems final: public Grid<Fems> { friend class Grid;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Fems (femera::Work::Core_ptrs_t) noexcept;
    Fems () =delete;//NOTE Use the constructor above.
  };
} }//end femera::grid:: namespace

#include "Fems.ipp"

//end FEMERA_HAS_GRID_FEMS_HPP
#endif
