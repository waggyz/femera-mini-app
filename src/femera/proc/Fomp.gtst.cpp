#include "../core.h"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
auto test_omp = femera::proc::Main ();

TEST( Fomp, Init ){// Initialize Fmpi with MPI already init by early_main(..)
  EXPECT_EQ( test_omp.init (nullptr,nullptr), 0 );
  test_omp.proc = &test_omp;
}
TEST( Fomp, TaskName ){
  //TODO get_task by enum?
  EXPECT_EQ( test_omp.get_task_raw ({0,0,0,0})->name, "OpenMP" );
}
TEST( Fomp, ProcN ){
  EXPECT_EQ( test_omp.get_task_raw ({0,0,0,0})->get_proc_n (), uint(2) );
}
TEST( Fomp, Exit ){// should NOT finalize MPI: not initialized by test_omp
  EXPECT_EQ( test_omp.exit (0), 0 );
}
