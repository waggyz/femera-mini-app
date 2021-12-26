#include "Fomp.hpp"

#include "gtest/gtest.h"


auto test_omp = femera::proc::Fomp();

TEST( Fomp, TaskName ){
  EXPECT_EQ( test_omp.name, "OpenMP" );
}
TEST( Fomp, SizeofGtstGE120 ){
  EXPECT_GE( sizeof(test_omp), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
