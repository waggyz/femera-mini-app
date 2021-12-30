#include "Jobs.hpp"

#include "gtest/gtest.h"

auto test_jobs = femera::sims::Jobs();

TEST( Jobs, TaskName ){
  EXPECT_EQ( test_jobs.name, "simulation job handler" );
}
TEST( Jobs, SizeofGtstGE120 ){
  EXPECT_GE( sizeof(test_jobs), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
