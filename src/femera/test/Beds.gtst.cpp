#include "Beds.hpp"

#include "gtest/gtest.h"


auto testbeds = femera::Beds();

TEST( Beds, TaskName ){
  EXPECT_EQ( testbeds.name, "testbeds" );
}
TEST( Beds, SizeofBedsGE120 ){
  EXPECT_GE( sizeof(testbeds), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
