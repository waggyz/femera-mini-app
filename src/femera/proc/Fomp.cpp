#include "Fomp.hpp"
#include "Main.hpp"
#include "../Data.hpp"

#ifdef _OPENMP
#include "omp.h"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  proc::Fomp::Fomp (const Work::Core_ptrs_t core) noexcept {
    std::tie (this->proc, this->data, this->test) = core;
    this->name ="OpenMP";
    this->abrv ="omp";
    this->version = std::to_string (_OPENMP);
    this->task_type = task_cast (Plug_type::Fomp);
    //this->proc_ix = this->task_proc_ix ();// set in parallel if >1 Fomp loaded
    this->proc_n  = fmr::Local_int (::omp_get_max_threads ());
  }
  void proc::Fomp::task_init (int*, char**) {
    if (true) {//TODO Handle command line options.
      this->proc_n = 2;//TODO This is for TDD
      ::omp_set_num_threads (int (this->proc_n));
      FMR_PRAGMA_OMP(omp parallel) {
        this->proc_n = fmr::Local_int (::omp_get_num_threads ());
      }
#if 0
      if (this->data != nullptr) {
        const auto name = this->data->text_line ("%4s %4s %4s",
          this->get_base_name ().c_str(), this->abrv.c_str(), "thrd");
        data->name_line (data->fmrall, name.c_str(), "%4u for process %4u",
          this->proc_n, this->proc->get_proc_id ());//TODO wrong before init done
      }
#endif
    }
#if 0
    if (this->is_in_parallel ()) {//TODO 1 Fomp/team or 1 Fomp/openmp thrd?
      this->proc_n = fmr::Local_int (::omp_get_num_threads ());
    } else {
      if (false && this->proc != nullptr) {//TODO calculate number of OpenMP threads
        this->proc->auto_proc_n ();// sets this->proc_n
        if (this->data != nullptr) {
          const auto name = this->data->text_line ("%4s %4s %4s",
            this->get_base_name ().c_str(), this->abrv.c_str(), "thrd");
          data->name_line (data->fmrlog, name.c_str(), "%u", this->proc_n);
      } }
      ::omp_set_num_threads (int (this->proc_n));
      FMR_PRAGMA_OMP(omp parallel) {
        this->proc_n = fmr::Local_int (::omp_get_num_threads ());
  } }
#endif
  }
  bool proc::Fomp::is_in_parallel () {
    return ::omp_in_parallel ();
  }
  fmr::Local_int proc::Fomp::task_proc_ix () {
#ifdef FMR_DEBUG
    printf ("%s Fomp::task_proc_ix %i\n", abrv.c_str(), ::omp_get_thread_num());
#endif
    return fmr::Local_int (::omp_get_thread_num ());
  }
}//end femera:: namespace

#undef FMR_DEBUG
