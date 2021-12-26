#include "Main.hpp"

#include "gtest/gtest.h"


auto fmr_main = femera::proc::Main();

TEST( Main, TaskName ){
  EXPECT_EQ( fmr_main.name, "Femera" );
}
TEST( Main, SizeofMainGE120 ){
  EXPECT_GE( sizeof(fmr_main), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
