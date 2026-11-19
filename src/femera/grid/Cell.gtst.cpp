#include "../femera.hpp"

#include <gtest/gtest.h>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}

TEST( GridCell, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

