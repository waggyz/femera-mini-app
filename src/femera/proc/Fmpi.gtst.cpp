#include "../core.h"

#include "mpi.h"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
auto test_mpi = femera::proc::Main ();

TEST( Fmpi, Init ){// Initialize Fmpi with MPI already init by early_main(..)
  EXPECT_EQ( test_mpi.init (nullptr,nullptr), 0 );
  test_mpi.proc = &test_mpi;
}
TEST( Fmpi, TaskName ){
  //TODO get_task by enum?
  EXPECT_EQ( test_mpi.get_task (femera::Plug_type::Fmpi)->get_abrv (), "mpi" );
}
TEST( Fmpi, TeamID ){
  const auto M = test_mpi.get_task (femera::Plug_type::Fmpi);
  EXPECT_NE( M->get_team_id (), uint(0) );
  EXPECT_NE( M->get_team_id (), femera::proc::Team_t (MPI_COMM_WORLD) );
}
TEST( Fmpi, ProcN ){
  const auto M = test_mpi.get_task (femera::Plug_type::Fmpi);
  EXPECT_EQ( M->get_proc_n (), uint(2) );
}
TEST( Fmpi, Exit ){// should NOT finalize MPI: not initialized by test_mpi
  EXPECT_EQ( test_mpi.exit (0), 0 );
  int is_init=0;
  int err = MPI_Initialized (& is_init);
  EXPECT_EQ( err, 0 );
  EXPECT_EQ( bool (is_init), true );
}
