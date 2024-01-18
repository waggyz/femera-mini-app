#include "femera.hpp"

#if 0
#include <gtest/gtest.h>
TEST( Mini, TrivialTest ){
  EXPECT_EQ( 0, 0 );
}
#endif
int main (int argc, char** argv) {
  // These all work.
  const int choice = 1;
switch ( choice ){
  case 1: {
    return fmr::new_jobs (& argc, argv)->exit ();// one-liner
  }
  case 2: {
    auto mini = fmr::new_jobs (& argc, argv);
    // Do mini.* things after parsing the arguments
    return mini->exit ();// exit argument defaults to 0.
  }
  case 3: {
    auto mini = fmr::new_jobs ();
    // Do mini.* things before parsing the arguments (not recommended)
    return mini->exit (mini->init (& argc, argv));
  }
  case 4: {
    auto mini = fmr::new_jobs ();
    // Do mini.* things before parsing the arguments (not recommended)
    auto err = mini->init (& argc, argv);
    // Do mini.* things after parsing the arguments
    return mini->exit (err);
} } }
