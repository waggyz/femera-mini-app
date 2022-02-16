#include "Fomp.hpp"

#ifdef FMR_HAS_OMP
#include "omp.h"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  proc::Fomp::Fomp (femera::Work::Core_ptrs core) noexcept {
    std::tie (this->proc, this->data, this->test) = core;
    this->name ="OpenMP";
    this->abrv ="omp";
    this->version = std::to_string( _OPENMP );
    this->task_type = task_cast (Plug_type::Fomp);
    this->base_type = task_cast (Base_type::Proc);
//    this->proc_ix = this->task_proc_ix ();
    this->proc_n  = fmr::Local_int (::omp_get_max_threads ());
  }
  void proc::Fomp::task_init (int*, char**) {
    if (this->is_in_parallel ()) {
      this->proc_n  = fmr::Local_int (::omp_get_num_threads ());
    } else {
      const int n = 2;//FIXME Handle command line options.
      ::omp_set_num_threads (n);
      FMR_PRAGMA_OMP(omp parallel) {
        this->proc_n  = fmr::Local_int (::omp_get_num_threads ());
  } } }
  bool proc::Fomp::is_in_parallel () {
    return ::omp_in_parallel ();
  }
  fmr::Local_int proc::Fomp::task_proc_ix () {
#ifdef FMR_DEBUG
    printf ("%s Fomp::task_proc_ix %i\n", abrv.c_str(), ::omp_get_thread_num());
#endif
    return fmr::Local_int (::omp_get_thread_num  ());
  }
#if 0
bool Fomp::is_in_parallel (){
  return is_omp_parallel ();
}
int Fomp::get_proc_ix (){
  return get_omp_thread_num ();
}
#endif
}//end femera:: namespace

#undef FMR_DEBUG
