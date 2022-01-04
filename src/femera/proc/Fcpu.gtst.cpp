#include "Fcpu.hpp"

#include "gtest/gtest.h"


auto test_mpi = femera::proc::Fcpu
  (femera::Work::Core_t (nullptr,nullptr,nullptr));

TEST( Fcpu, TaskName ){
  EXPECT_EQ( test_mpi.name, "CPU" );
}
TEST( Fcpu, SizeofGtstGE120 ){
  EXPECT_GE( sizeof(test_mpi), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
