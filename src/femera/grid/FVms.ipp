#ifndef FEMERA_HAS_GRID_FVMS_IPP
#define FEMERA_HAS_GRID_FVMS_IPP

namespace femera {

inline
grid::FVms::FVms (const femera::Work::Core_ptrs_t core)
noexcept : Grid (core) {
  this->name      ="Femera finite volume grid";
  this->abrv      ="FV";
  this->task_type = task_cast (Task_type::FVms);
}
inline
void grid::FVms::task_init (int*, char**) {
  this->set_init (true);
}
inline
void grid::FVms::task_exit () {
}

}//end femera namespace

//end FEMERA_HAS_GRID_FVMS_IPP
#endif
