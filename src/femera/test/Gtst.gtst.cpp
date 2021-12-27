#include "Gtst.hpp"

#include "gtest/gtest.h"

auto gtst = femera::test::Gtst();

TEST( Gtst, TaskName ){
  EXPECT_EQ( gtst.name, "GoogleTest" );
}
TEST( Gtst, SizeofGtstGE120 ){
  EXPECT_GE( sizeof(gtst), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
