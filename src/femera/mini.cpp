#include "femera.hpp"

#if 0
#include "gtest/gtest.h"
TEST( Mini, TrivialTest ){
  EXPECT_EQ( 0, 0 );
}
#endif
int main (int argc, char** argv) {
  // These all work.
#if 0
  auto mini = fmr::new_jobs ();
  return mini->exit (mini->init (& argc, argv));
#endif
#if 0
  auto mini = fmr::new_jobs (& argc, argv);
  return mini->exit ();
#endif
#if 1
  return fmr::new_jobs (& argc, argv)->exit ();// one-liner
#endif
}
