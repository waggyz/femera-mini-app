#include "gtest/gtest.h"

//#include "../core.h"
#include "../Main/Plug.hpp"
#include "Gtst.hpp"

#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif


//NOTE Do not use underscores in suite and test names.

TEST( WorkGoogletest, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
int main(int argc, char **argv) {
#ifdef FMR_HAS_MPI
  int err=0;
  ::MPI_Init( &argc,&argv );
  ::testing::InitGoogleTest( &argc,argv );
  int proc_id=0; ::MPI_Comm_rank( MPI_COMM_WORLD,& proc_id );
  //
  //from: https://github.com/google/googletest/issues/822
  ::testing::TestEventListeners& listeners =
  ::testing::UnitTest::GetInstance()->listeners();
  if( !proc_id ){// Only print from master
    delete listeners.Release(listeners.default_result_printer());
  }
  err = RUN_ALL_TESTS();
  ::MPI_Finalize();
#if 0
  if( MPI_VERSION < 3 ){ return 0; }//TODO Make MPI exit cleanly.
#endif
  return err;
#else
  ::testing::InitGoogleTest( &argc,argv );
  return RUN_ALL_TESTS();
#endif
}
