#include "femera.hpp"

fmr::Jobs_spt fmr::new_sims (){
  return femera::Sims<femera::sims::Jobs>::new_task ();
}
#if 0
fmr::Jobs_spt fmr::new_sims (int* argc, char** argv){
  auto jobs = femera::new_sims ();
  jobs->init (argc,argv);
  return jobs;
}
#else
fmr::Jobs_spt fmr::new_sims (int* argc, char** argv){
  return femera::Sims<femera::sims::Jobs>::new_task (argc,argv);
}
#endif
