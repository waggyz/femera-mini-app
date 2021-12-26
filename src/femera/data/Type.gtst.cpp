#include "Type.hpp"

#include "gtest/gtest.h"


auto testbeds = femera::data::Type();

TEST( Type, TaskName ){
  EXPECT_EQ( testbeds.name, "data" );
}
TEST( Type, SizeofTypeGE120 ){
  EXPECT_GE( sizeof(testbeds), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
