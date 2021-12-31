#include "femera.hpp"

int main (int argc, char** argv) {
#if 0
  //auto mini = femera::Sims<femera::sims::Jobs> ();
  auto mini = femera::Proc::new_main ();
  return mini.exit (mini.init (&argc, argv));
#endif
//  auto mini = femera::Proc<femera::Main>::new_task ();//FIXME replace w/below
  //
  //TODO provide below as femera::new_sims() and femera::new_sims(int, char**)
//  auto mini = femera::Sims<femera::sims::Jobs>::new_task ();
  auto mini = femera::new_sims();
}
