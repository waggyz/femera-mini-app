#include "femera.hpp"

#if 0
#include <gtest/gtest.h>
TEST( Mini, TrivialTest ){
  EXPECT_EQ( 0, 0 );
}
#endif
int main (int argc, char** argv) {
  const int choice = 0;
  switch ( choice ){
    // These all work.
    case 0: {
      return fmr::new_jobs (& argc, argv)->exit ();// one-liner
    }
    case 1: {// Preferred
      const auto mini_ptr = std::move (fmr::new_jobs (& argc, argv));
      // --or-- auto mini_ptr = fmr::new_jobs (& argc, argv);
      auto& mini = *mini_ptr;
      // Do mini.* things after parsing the arguments
      return mini.exit ();
    }
    case 2: {
      const auto mini = std::move (fmr::new_jobs ());
      // Do mini->* things before parsing the arguments (not recommended)
      auto err = mini->init (& argc, argv);
      // Do mini->* things after parsing the arguments
      return mini->exit (err);
    }
    case 3: {
      auto mini = fmr::new_jobs (& argc, argv);
      // Do mini->* things after parsing the arguments
      return mini->exit ();// exit argument defaults to 0.
    }
    case 4: {
      auto mini = fmr::new_jobs ();
      // Do mini->* things before parsing the arguments (not recommended)
      return mini->exit (mini->init (& argc, argv));
    }
    case 5: {
      auto mini = fmr::new_jobs ();
      // Do mini->* things before parsing the arguments (not recommended)
      auto err = mini->init (& argc, argv);
      // Do mini->* things after parsing the arguments
      return mini->exit (err);
    }
    case 6: {// segfaults
      auto& mini = *fmr::new_jobs ();
      // Do mini.* things before parsing the arguments (not recommended)
      auto err = mini.init (& argc, argv);
      // Do mini.* things after parsing the arguments
      return mini.exit (err);
    }
    case 7: {// segfaults
      auto& mini = *std::move(fmr::new_jobs ());
      // Do mini.* things before parsing the arguments (not recommended)
      auto err = mini.init (& argc, argv);
      // Do mini.* things after parsing the arguments
      return mini.exit (err);
    }
    default: return 1;
} }
