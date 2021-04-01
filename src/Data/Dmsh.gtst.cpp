#include "gtest/gtest.h"
#include "../Main/Plug.hpp"
//NOTE Tests assume:
// mpiexec -np <mpi_n> -bind-to core -map-by node:pe=<omp_n> ompexec *.gtest
using namespace Femera;

TEST( Dmsh, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}

int main(int argc, char** argv ){
  // gtest run_all_tests is done during fmr::exit(err).
  return fmr:: exit ( fmr:: init (&argc,argv) );
}
