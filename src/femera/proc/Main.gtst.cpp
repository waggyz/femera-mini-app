#include "Main.hpp"

#include "gtest/gtest.h"

auto fmr_main = std::make_shared<femera::proc::Main> (femera::proc::Main());

TEST( Main, TaskName ){
  EXPECT_EQ( fmr_main->name, "Femera");
}
TEST( Main, SizeofMainGE120 ){
  EXPECT_GE( sizeof(*fmr_main), 120 );
}
TEST( Main, Init ){
  EXPECT_EQ( fmr_main, fmr_main->proc );
  EXPECT_EQ( fmr_main->proc->name, "Femera");
}
fmr::Exit_int main (int argc, char** argv) {
  fmr_main->proc = fmr_main;
  fmr_main->init (&argc,argv);
  return femera::test:: early_main (&argc, argv);
}
