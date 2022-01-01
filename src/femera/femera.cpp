#include "femera.hpp"

femera::Jobs_t femera::new_sims (){
  return femera::Sims<femera::sims::Jobs>::new_task ();
}
#if 0
femera::Jobs_t femera::new_sims (int* argc, char** argv){
  auto jobs = femera::new_sims ();
  jobs->init (argc,argv);
  return jobs;
}
#else
femera::Jobs_t femera::new_sims (int* argc, char** argv){
  return femera::Sims<femera::sims::Jobs>::new_task (argc,argv);
}
#endif
