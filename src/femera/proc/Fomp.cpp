#include "Fomp.hpp"

#ifdef FMR_HAS_OMP
#include "omp.h"
#endif

#if 0
bool Fomp::is_in_parallel (){
  return omp_in_parallel ();
}
int Fomp::get_proc_ix (){
  return omp_get_thread_num ();
}
#endif
