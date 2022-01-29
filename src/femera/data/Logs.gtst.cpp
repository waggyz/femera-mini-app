#include "../femera.hpp"

#include "gtest/gtest.h"

auto mini = femera::new_sims ();
fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}

TEST( Mini, DataName ){
  EXPECT_EQ( mini->data->name, "log" );
}
