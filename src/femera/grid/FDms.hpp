#ifndef FEMERA_HAS_GRID_FDMS_HPP
#define FEMERA_HAS_GRID_FDMS_HPP

#include "../Grid.hpp"

namespace femera { namespace grid {
  class FDms;// Derive a CRTP concrete class from Grid.
  class FDms final: public Grid<FDms> { friend class Grid;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    FDms (femera::Work::Core_ptrs_t) noexcept;
    FDms () =delete;//NOTE Use the constructor above.
  };
} }//end femera::grid:: namespace

#include "FDms.ipp"

//end FEMERA_HAS_GRID_FDMS_HPP
#endif
