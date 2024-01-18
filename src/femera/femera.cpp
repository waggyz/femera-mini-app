#include "femera.hpp"

fmr::Jobs_t fmr::new_jobs () {
  return femera::Task<femera::task::Jobs>::new_task ();
}
fmr::Jobs_t fmr::new_jobs (int* argc, char** argv) {
  return femera::Task<femera::task::Jobs>::new_task (argc,argv);
}
