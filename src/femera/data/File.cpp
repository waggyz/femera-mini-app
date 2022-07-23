#include "Logs.hpp"
#include "Text.hpp"
#include "Dlim.hpp"

#include "Bank.hpp"

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

#include <array>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  data::File::File (const Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name      ="Femera file handler";
    this->abrv      ="file";
    this->task_type = task_cast (Task_type::File);
    this->info_d    = 2;
  }
  void data::File::task_init (int*, char**) {
//    if (this->data == nullptr) {this->data = this;}
    fmr::Local_int o = 1;
#ifdef FMR_BANK_LOCAL
    o = this->proc->get_proc_n (Task_type::Fomp);
#endif
FMR_WARN_INLINE_OFF
    FMR_PRAGMA_OMP(omp parallel for schedule(static) ordered num_threads(o))
    for (fmr::Local_int i=0; i<o; ++i) {// Make & add thread-local data::Bank
      FMR_PRAGMA_OMP(omp ordered) {     // in order.
        auto B = Data<data::Bank>::new_task (this->get_core());
#ifdef FMR_BANK_LOCAL
        B->set_name ("Femera data bank for process "
          + std::to_string (this->proc->get_proc_id ()));
#endif
#ifdef FMR_DEBUG
        const auto m = this->proc->get_proc_ix (Task_type::Fmpi);
        printf ("%s (MPI: %u)\n", B->get_name ().c_str(), m);
#endif
        this->add_task (std::move (B));
    } }
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
FMR_WARN_INLINE_ON
  }
#if 0
  bool data::File::does_file (const fmr::Data_name_NEW_t& file)
  noexcept {
    if (! this->task_list.empty ()) {
      const fmr::Local_int n = this->get_task_n ();
      for (fmr::Local_int i=0; i < n; ++i) {
        auto D = this->get_task (i);
#if 1//def FMR_DEBUG
        printf ("task type %u\n", fmr::Enum_int (D->task_type));
#endif
        switch (fmr::Enum_int (D->task_type)) {
          case fmr::Enum_int (Task_type::Logs) : return true;
    } } }
    return false;
  }
#endif
  std::size_t data::File::NEW_send
  (const fmr::Data_name_NEW_t& file, const std::string& text)
  noexcept { size_t byte = 0;
    if (! this->task_list.empty ()) {// only checks top-level tasks
      const fmr::Local_int n = this->get_task_n ();
      for (fmr::Local_int Wix=0; Wix < n; ++Wix) {
        bool ok = false;
        const auto D = this->get_task (Wix);
        switch (fmr::Enum_int (D->task_type)) {
          case fmr::Enum_int (Task_type::Logs): {
            auto C = Work::cast_via_work <data::Logs> (D);
            ok = C->does_file (file);
            if (ok) {
              byte += text.size();//byte += C->send (file, text);
          } }
          default: {}// do nothing
        }
#if 1//def FMR_DEBUG
        printf ((D->get_abrv()
          +( ok ? " handles " : " does not handle ")
          +file+"\n").c_str());
#endif
    } }
    return byte;
  }
}// end femera::namespace

#undef FMR_DEBUG
