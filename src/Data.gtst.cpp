#include "gtest/gtest.h"
#include "base.h"
using namespace Femera;

TEST( Data, SizeofDataGE416 ){
  EXPECT_GE( sizeof(Data), 416 );
}
#if 0
TEST( Data, TestHas6Sims ){
  EXPECT_EQ( fmr::get_sims_n(), 6 );
}
#endif
#if 0
TEST( Data, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
#endif
#if 0
TEST( DataDummy, ReturnsNonzero ){
  EXPECT_NE(  0     , data_inline_dumm());
#endif
int main(int argc, char** argv ){
  // gtest run_all_tests is done during fmr::exit(err).
  return fmr::exit (fmr::init (&argc,argv) );// These are being tested
}
