#include "File.hpp"
#include "Logs.hpp"
#include "Text.hpp"
#include "Fcsv.hpp"

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
    this->add_task (std::move (Data<data::Logs>::new_task (this->get_core())));
    this->add_task (std::move (Data<data::Text>::new_task (this->get_core())));
    this->add_task (std::move (Data<data::Fcsv>::new_task (this->get_core())));
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
  bool data::File::set_logs_init (const bool tf) {
    this->logs_init_stat = tf;
#if 0
    if (tf) {
      auto D = cast_via_work<data::Text>(this->get_task (Plug_type::Text));
      if (D != nullptr) {
        D->file_line_sz = this->file_line_sz;
        D->line_name_sz = this->line_name_sz;
    } }
#endif
    return this->logs_init_stat;
  }
}// end femera::namespace

#undef FMR_DEBUG
