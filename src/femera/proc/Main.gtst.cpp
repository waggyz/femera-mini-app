#include "Main.hpp"

#include "gtest/gtest.h"

auto fmr_main = FMR_MAKE_SMART<femera::proc::Main> (femera::proc::Main());

TEST( Main, TaskName ){
  EXPECT_EQ( fmr_main->name, "processing");
  //xxx_EXPECT_EQ( fmr_main->get_task_raw({})->name, "processing");
}
TEST( Main, DidInit ){
  EXPECT_EQ( fmr_main->get_task_n(), uint(1) );
  EXPECT_EQ( fmr_main->get_task_raw(0)->name, "root process");
  EXPECT_EQ( fmr_main->get_task_raw({0})->name, "root process");
  EXPECT_NE( fmr_main->get_task_raw({0,0})->name, "node");
#ifdef FMR_HAS_MPI
  EXPECT_EQ( fmr_main->get_task_raw({0,0,0})->abrv, "mpi");
#endif
//  EXPECT_EQ( fmr_main->exit (), 0 );
}
int main (int argc, char** argv) {
  fmr_main->init (&argc,argv);
  return femera::test:: early_main (& argc, argv);
}
