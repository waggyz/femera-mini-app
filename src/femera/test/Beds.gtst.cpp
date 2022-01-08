#include "Beds.hpp"

#include "gtest/gtest.h"


auto testbeds = femera::test::Beds
 ( femera::Work::Core_t (nullptr,nullptr,nullptr) );

TEST( Beds, TaskName ){
  EXPECT_EQ( testbeds.name, "testbeds" );
}
TEST( Beds, SizeofBedsGEWork ){
  EXPECT_GE( sizeof(testbeds), sizeof(femera::Work) );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
