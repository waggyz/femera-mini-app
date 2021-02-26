#include "gtest/gtest.h"
#include "../base.h"

using namespace Femera;
#if 1
TEST( DataFake, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
#endif

int main(int argc, char** argv ){
  // gtest run_all_tests() is done during frm::exit(err).
  return fmr::exit (fmr::init (&argc,argv) );
}
