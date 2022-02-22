#include "../core.h"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

#ifdef FMR_HAS_NVIDIA
auto test_main = femera::proc::Main ();

TEST( Nvid, Init ){// Initialize Fmpi with MPI already init by early_main(..)
  EXPECT_EQ( test_main.init (nullptr,nullptr), 0 );
  test_main.proc = & test_main;
}
#if 0
TEST( Nvid, TaskName ){
  //FIXME get_task by enum?
  EXPECT_EQ( test_main.get_task_raw ({0,0,0,1})->abrv, "gpu" );
}
TEST( Nvid, ProcN ){
  EXPECT_GT( test_main.get_task_raw ({0,0,0,1})->get_proc_n (), uint(0) );
}
#endif
TEST( Nvid, Exit ){// should NOT finalize MPI: not initialized by test_main
  EXPECT_EQ( test_main.exit (0), 0 );
}
#else
TEST( NvidNotUsed, TrivialTest ){
  EXPECT_EQ( 0, 0 );
}
#endif
