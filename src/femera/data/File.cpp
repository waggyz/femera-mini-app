#include "File.hpp"
#include "Logs.hpp"
#include "Text.hpp"
#include "Dlim.hpp"

#include "Vals.hpp"

#ifdef FMR_HAS_CGNS
#include "Cgns.hpp"
#endif
#ifdef FMR_HAS_MOAB
#include "Moab.hpp"
#endif
#ifdef FMR_HAS_GMSH
#include "Gmsh.hpp"
#endif
#ifdef FMR_HAS_PETSC
#include "Pets.hpp"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void data::File::task_init (int*, char**) {
    fmr::Local_int n = 1;
#ifdef FMR_VALS_LOCAL
    if (this->proc != nullptr) {
      const auto P = this->proc->get_task (Plug_type::Fomp);
      if (P != nullptr) {
        n = P->get_proc_n ();
    } }
#endif
    FMR_PRAGMA_OMP(omp parallel for schedule(static) ordered num_threads(n))
    for (fmr::Local_int i=0; i<n; i++) {// add thread-local Vals
      FMR_PRAGMA_OMP(omp ordered)
      this->add_task (std::move (Data<data::Vals>::new_task(this->get_core())));
    }
    this->add_task (std::move (Data<data::Logs>::new_task (this->get_core())));
    this->add_task (std::move (Data<data::Text>::new_task (this->get_core())));
    this->add_task (std::move (Data<data::Dlim>::new_task (this->get_core())));
#ifdef FMR_HAS_CGNS
    this->add_task (std::move (Data<data::Cgns>::new_task (this->get_core())));
#endif
#ifdef FMR_HAS_MOAB
    this->add_task (std::move (Data<data::Moab>::new_task (this->get_core())));
#endif
#ifdef FMR_HAS_GMSH
    this->add_task (std::move (Data<data::Gmsh>::new_task (this->get_core())));
#endif
#ifdef FMR_HAS_PETSC
    this->add_task (std::move (Data<data::Pets>::new_task (this->get_core())));
#endif
  }
}// end femera::namespace

#undef FMR_DEBUG
