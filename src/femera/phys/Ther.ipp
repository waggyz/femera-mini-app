#ifndef FEMERA_HAS_PHYS_THER_IPP
#define FEMERA_HAS_PHYS_THER_IPP

namespace femera {

inline
phys::Ther::Ther (const femera::Work::Core_ptrs_t core)
noexcept : Phys (core) {
  this->name      ="Femera thermal physics";
  this->abrv      ="ther";
  this->task_type = task_cast (Task_type::Ther);
}
inline
void phys::Ther::task_init (int*, char**) {
  this->set_init (true);
}
inline
void phys::Ther::task_exit () {
}

}//end femera namespace

//end FEMERA_HAS_PHYS_THER_IPP
#endif
