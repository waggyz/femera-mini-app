#ifndef FEMERA_HAS_GRID_FVMS_HPP
#define FEMERA_HAS_GRID_FVMS_HPP

#include "../Grid.hpp"

namespace femera { namespace grid {

class FVms;// Derive a CRTP concrete class from Grid.
class FVms final: public Grid<FVms> { friend class Grid;
private:
  void task_init (int* argc, char** argv);
  void task_exit ();
private:
  FVms (femera::Work::Core_ptrs_t) noexcept;
  FVms () =delete;//NOTE Use the constructor above.
};

} }//end femera::grid:: namespace

#include "FVms.ipp"

//end FEMERA_HAS_GRID_FVMS_HPP
#endif
