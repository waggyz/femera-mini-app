#include "gtest/gtest.h"
#include "../base.h"

using namespace Femera;
#if 1
TEST( Frun, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
#endif

int main(int argc, char** argv ){
  // gtest run_all_tests() is done during fmr::exit(err).
  return fmr::exit (fmr::init (&argc,argv) );
}
