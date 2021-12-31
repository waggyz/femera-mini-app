#include "femera.hpp"
#include "sims/Jobs.hpp"

femera::Jobs_t femera::new_sims (){
  return femera::Sims<femera::sims::Jobs>::new_task ();
}
#if 0
femera::Sims_t femera::new_sims (int, char**){
}
#endif
