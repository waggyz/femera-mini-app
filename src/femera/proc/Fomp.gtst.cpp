#include "../core.h"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (& argc, argv);
}
auto test_main = femera::proc::Main ();

#ifdef FMR_HAS_MPI
femera::Work::Task_path_t path_omp = {0,0,0,0};
#else
femera::Work::Task_path_t path_omp = {0,0,0};
#endif

TEST( Fomp, Init ){// Initialize Fmpi with MPI already init by early_main (..)
  EXPECT_EQ( 0, test_main.init (nullptr,nullptr) );
  test_main.proc = & test_main;
}
TEST( Fomp, TaskAbrv ){
  //TODO get_task by enum?
  EXPECT_EQ( test_main.get_task_raw (path_omp)->abrv, "omp" );
}
TEST( Fomp, TddProcN ){
  EXPECT_EQ( test_main.get_task_raw
    (path_omp)->get_proc_n (), fmr::Local_int (2) );
}
TEST( Fomp, Exit ){// should NOT finalize MPI: not initialized by test_main
  EXPECT_EQ( test_main.exit (0), 0 );
}
