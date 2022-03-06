#include "File.hpp"
#include "Logs.hpp"

#ifdef FMR_HAS_CGNS
#include "Cgns.hpp"
#endif
#ifdef FMR_HAS_MOAB
#include "Moab.hpp"
#endif
#ifdef FMR_HAS_GMSH
#include "Gmsh.hpp"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void data::File::task_init (int*, char**) {
    this->add_task (std::move(Data<data::Logs>::new_task (this->get_core())));
#ifdef FMR_HAS_CGNS
    this->add_task (std::move(Data<data::Cgns>::new_task (this->get_core())));
#endif
#ifdef FMR_HAS_MOAB
    this->add_task (std::move(Data<data::Moab>::new_task (this->get_core())));
#endif
#ifdef FMR_HAS_GMSH
    this->add_task (std::move(Data<data::Gmsh>::new_task (this->get_core())));
#endif
  }
}// end femera::namespace

#undef FMR_DEBUG
