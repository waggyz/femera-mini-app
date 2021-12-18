#include "mpi.h"
#include "gtest/gtest.h"

//#include "core.h"

//FIXME below
#include "Work.hpp"

using namespace femera;

// Work is abstract (pure virtual) and cannot be instantiated for testing.
auto fmr = Main();

TEST( fmr, TaskName ){
  EXPECT_EQ( work->task_name, "Main" );
}
TEST( fmr, SizeofWorkGE120 ){
  EXPECT_GE( sizeof(fmr), 120 );
}
int main(int argc, char **argv) {
  int err=0;
#if 1 //def FMR_HAS_MPI
  ::MPI_Init( &argc,&argv );
  ::testing::InitGoogleTest( &argc,argv );
  int proc_id=0; ::MPI_Comm_rank( MPI_COMM_WORLD,& proc_id );
  //
  //from: https://github.com/google/googletest/issues/822
  ::testing::TestEventListeners& listeners =
  ::testing::UnitTest::GetInstance()->listeners();
  if( proc_id!=0 ){// Only print from master
    delete listeners.Release(listeners.default_result_printer());
  }
  err = RUN_ALL_TESTS();
  if(err){
    if(proc_id==0){
      printf("  WARN  GoogleTest returned %i\n",err);
    }
    err=0;
  }
  ::MPI_Finalize();
#else
  ::testing::InitGoogleTest( &argc,argv );
  err= RUN_ALL_TESTS();
#endif
  //delete work;
  return err;
}
#if 0
int main(int, char**){return 0; }
#endif
