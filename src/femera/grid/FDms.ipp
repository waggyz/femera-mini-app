#ifndef FEMERA_HAS_GRID_FDMS_HPP
#define FEMERA_HAS_GRID_FDMS_HPP

namespace femera {

inline
grid::FDms::FDms (const femera::Work::Core_ptrs_t core)
noexcept : Grid (core) {
  this->name      ="Femera finite difference grid";
  this->abrv      ="FD";
  this->task_type = task_cast (Task_type::FDms);
}
inline
void grid::FDms::task_init (int*, char**) {
  this->set_init (true);
}
inline
void grid::FDms::task_exit () {
}

}//end femera namespace

//end FEMERA_HAS_GRID_FDMS_HPP
#endif
