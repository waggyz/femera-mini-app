#include "Beds.hpp"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
auto testbeds = femera::test::Beds
 ( femera::Work::Core_t (nullptr,nullptr,nullptr) );

TEST( Beds, TaskName ){
  EXPECT_EQ( testbeds.name, "testbeds" );
}
