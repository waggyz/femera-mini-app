#include "../femera.hpp"

#include "gtest/gtest.h"

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}

TEST( Sims, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
