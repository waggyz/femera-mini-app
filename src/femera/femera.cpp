#include "femera.hpp"

fmr::Jobs_t fmr::new_jobs (){
  return femera::Task<femera::task::Jobs>::new_task ();
}
#if 0
fmr::Jobs_spt fmr::new_jobs (int* argc, char** argv){
  auto jobs = femera::new_jobs ();
  jobs->init (argc,argv);
  return jobs;
}
#else
fmr::Jobs_t fmr::new_jobs (int* argc, char** argv){
  return femera::Task<femera::task::Jobs>::new_task (argc,argv);
}
#endif
