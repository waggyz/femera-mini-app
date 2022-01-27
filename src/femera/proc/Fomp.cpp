#include "Fomp.hpp"

#ifdef FMR_HAS_OMP
#include "omp.h"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  proc::Fomp::Fomp (femera::Work::Core_t core) noexcept {
    std::tie (this->proc, this->data, this->test) = core;
    this->name ="OpenMP";
    this->abrv ="omp";
    this->proc_ix = fmr::Local_int (::omp_get_thread_num  ());
    this->proc_n  = fmr::Local_int (::omp_get_max_threads ());
#ifdef FMR_DEBUG
    printf ("Fomp::proc_ix %u\n", this->proc_ix);
#endif
  }
#if 0
bool Fomp::is_in_parallel (){
  return omp_in_parallel ();
}
int Fomp::get_proc_ix (){
  return omp_get_thread_num ();
}
#endif
}//end femera:: namespace

#undef FMR_DEBUG
