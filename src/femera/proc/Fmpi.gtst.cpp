#include "Fmpi.hpp"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

auto test_mpi = femera::proc::Fmpi
  (femera::Work::Core_t(nullptr,nullptr,nullptr));

TEST( Fmpi, TaskName ){
  EXPECT_EQ( test_mpi.name, "MPI" );
}
TEST( Fmpi, SizeofGtstGEWork ){
  EXPECT_GE( sizeof(test_mpi), sizeof(femera::Work) );
}
