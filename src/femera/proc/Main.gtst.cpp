#include "Main.hpp"

#include "gtest/gtest.h"

auto fmr_main = FMR_MAKE_SMART<femera::proc::Main> (femera::proc::Main());

TEST( Main, TaskName ){
  EXPECT_EQ( fmr_main->abrv, "main");
  //xxx_EXPECT_EQ( fmr_main->get_task_raw({})->abrv, "main");
}
TEST( Main, DidInit ){
  EXPECT_EQ( fmr_main->get_task_n(), uint(1) );
  EXPECT_EQ( fmr_main->get_task_raw(0)->abrv, "root");
  EXPECT_EQ( fmr_main->get_task_raw({0})->abrv, "root");
  EXPECT_EQ( fmr_main->get_task_raw({0,0})->abrv, "node");
#ifdef FMR_HAS_MPI
  EXPECT_EQ( fmr_main->get_task_raw({0,0,0})->abrv, "mpi");
#endif
//  EXPECT_EQ( fmr_main->exit (), 0 );
}
int main (int argc, char** argv) {
  fmr_main->init (&argc,argv);
  return femera::test:: early_main (& argc, argv);
}
