#include "gtest/gtest.h"
#include "base.h"

using namespace Femera;
#if 0
TEST( sims, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
#endif
TEST( Sims, SizeofSimsGE216 ){
  EXPECT_GE( sizeof(Sims), 216 );
}

int main(int argc, char** argv ){
  // gtest run_all_tests() is done during frm::exit(err).
  return fmr::exit (fmr::init (&argc,argv) );
}
