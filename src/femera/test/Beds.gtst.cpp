#include "../femera.hpp"

#include "gtest/gtest.h"

auto mini = fmr::new_sims ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
TEST( Gtst, TaskName ){
  EXPECT_EQ( mini->test->name, "testbeds" );
}
