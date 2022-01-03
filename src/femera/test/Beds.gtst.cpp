#include "Beds.hpp"

#include "gtest/gtest.h"


auto testbeds = femera::test::Beds
 ( femera::Work::Core_t (nullptr,nullptr,nullptr) );

TEST( Beds, TaskName ){
  EXPECT_EQ( testbeds.name, "testbeds" );
}
TEST( Beds, SizeofBedsGE120 ){
  EXPECT_GE( sizeof(testbeds), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
