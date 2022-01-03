#include "Main.hpp"

#include "gtest/gtest.h"

auto fmr_main = std::make_shared<femera::proc::Main> (femera::proc::Main());

TEST( Main, TaskName ){
  EXPECT_EQ( fmr_main->name, "processing environment");
}
TEST( Main, SizeofMainGE120 ){
  EXPECT_GE( sizeof(*fmr_main), 120 );
}
TEST( Main, DidInit ){
  EXPECT_EQ( fmr_main->get_task_n(), 1 );
  EXPECT_EQ( fmr_main->get_task(0)->name, "top");
  EXPECT_EQ( fmr_main->get_task({0})->name, "top");
#ifdef FMR_HAS_MPI
  EXPECT_EQ( fmr_main->get_task({0,0})->name, "MPI");
#endif
  EXPECT_EQ( fmr_main->exit (), 0 );
}
int main (int argc, char** argv) {
  fmr_main->init (&argc,argv);
  return femera::test:: early_main (& argc, argv);
}
