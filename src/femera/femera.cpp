#include "femera.hpp"
#include "sims/Jobs.hpp"

femera::Jobs_t femera::new_sims (){
  return femera::Sims<femera::sims::Jobs>::new_task ();
}
femera::Jobs_t femera::new_sims (int* argc, char** argv){
  auto jobs = femera::new_sims ();
  jobs->init (argc,argv);
  return jobs;
}
#if 0
#endif
