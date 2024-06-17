#include "femera.hpp"

fmr::Jobs_t fmr::new_jobs () {
FMR_WARN_INLINE_OFF
  return femera::Task<femera::task::Jobs>::new_task ();
FMR_WARN_INLINE_ON
}
fmr::Jobs_t fmr::new_jobs (int* argc, char** argv) {
  return femera::Task<femera::task::Jobs>::new_task (argc,argv);
}
