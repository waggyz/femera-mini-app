#include "../femera.hpp"

#include "gtest/gtest.h"

auto mini = femera::new_sims ();
fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
TEST( SelfTestProc, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
