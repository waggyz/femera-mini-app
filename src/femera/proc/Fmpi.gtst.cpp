#include "Fmpi.hpp"
//#include "../femera.hpp"
#include "mpi.h"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
//  return femera::new_sims (&argc,argv)->exit ();
}

auto test_mpi = femera::proc::Fmpi
  (femera::Work::Core_t(nullptr,nullptr,nullptr));

TEST( Fmpi, TaskName ){
  EXPECT_EQ( test_mpi.name, "MPI" );
}
TEST( Fmpi, Init ){// Initialize Fmpi with MPI already init by early_main(..)
  EXPECT_EQ( test_mpi.init (nullptr,nullptr), 0 );
}
TEST( Fmpi, TeamN ){// This Fmpi instance has no sub-tasks.
  EXPECT_EQ( test_mpi.get_team_n (), 0 );
}
TEST( Fmpi, ProcN ){
  EXPECT_EQ( test_mpi.get_proc_n (), 2 );
}
TEST( Fmpi, TeamID ){
  EXPECT_NE( test_mpi.get_team_id (), 0 );
  EXPECT_NE( test_mpi.get_team_id (), femera::proc::Team_t (MPI_COMM_WORLD) );
}
TEST( Fmpi, Exit ){// should NOT finalize MPI
  EXPECT_EQ( test_mpi.exit (0), 0 );
}
TEST( Fmpi, SizeofFmpiGEWork ){
  EXPECT_GE( sizeof(test_mpi), sizeof(femera::Work) );
}
