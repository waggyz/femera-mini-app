#include "Main.hpp"

#include "gtest/gtest.h"

auto fmr_main = FMR_MAKE_SMART(femera::proc::Main) (femera::proc::Main());

TEST( Main, TaskName ){
  EXPECT_EQ( fmr_main->get_abrv (), "main");
  EXPECT_EQ( fmr_main->get_task
    (femera::Work::Task_path_t())->get_abrv (), "main");
}
TEST( Main, DidInit ){
  EXPECT_EQ( fmr_main->get_task_n(), uint(1) );
#if 0
  EXPECT_EQ( fmr_main->get_task(femera::Plug_type::Root)->get_abrv (), "root");
#endif
  EXPECT_EQ( fmr_main->get_task(femera::Plug_type::Node)->get_abrv (), "node");
#ifdef FMR_HAS_MPI
  EXPECT_EQ( fmr_main->get_task(femera::Plug_type::Fmpi)->get_abrv (), "mpi");
#endif
}
int main (int argc, char** argv) {
  fmr_main->init (& argc, argv);
  return femera::test:: early_main (& argc, argv);
}
