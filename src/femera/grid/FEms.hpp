#ifndef FEMERA_HAS_GRID_FEMS_HPP
#define FEMERA_HAS_GRID_FEMS_HPP

#include "../Grid.hpp"

namespace femera { namespace grid {
  class FEms;// Derive a CRTP concrete class from Grid.
  class FEms final: public Grid<FEms> { friend class Grid;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    FEms (femera::Work::Core_ptrs_t) noexcept;
    FEms () =delete;//NOTE Use the constructor above.
  };
} }//end femera::grid:: namespace

#include "FEms.ipp"

//end FEMERA_HAS_GRID_FEMS_HPP
#endif
