#include "gtest/gtest.h"
#include "../core.h"
#if 0
using namespace Femera;
#if 0
TEST( sims, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
#endif
TEST( Sims, SizeofSimsGE224 ){
  EXPECT_GE( sizeof(Sims), 224 );
}

int main(int argc, char** argv ){
  // gtest run_all_tests is done during fmr::exit(err).
  return fmr::exit (fmr::init (&argc,argv) );
}
#else
int main(int, char**){return 0; }
#endif
