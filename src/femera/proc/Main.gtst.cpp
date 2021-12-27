#include "Main.hpp"
#include "Fmpi.hpp"
#include "Fomp.hpp"
#include "Fcpu.hpp"

#include "gtest/gtest.h"

auto fmr_main = std::make_shared<femera::Main> (femera::Main());

TEST( Main, TaskName ){
  EXPECT_EQ( fmr_main->name, "Femera");
}
TEST( Main, SizeofMainGE120 ){
  EXPECT_GE( sizeof(*fmr_main), 120 );
}
TEST( Main, DidInit ){
  EXPECT_EQ( fmr_main, fmr_main->proc );
  EXPECT_EQ( fmr_main->proc->name, "Femera");
  EXPECT_GT( fmr_main->get_task_n(), 0 );
#ifdef FMR_HAS_MPI
  EXPECT_EQ( fmr_main->get_task(0)->name, "MPI");
#endif
}
fmr::Exit_int main (int argc, char** argv) {
  fmr_main->proc = fmr_main;
#ifdef FMR_HAS_MPI
  fmr_main->add_task (std::make_shared<femera::proc::Fmpi> (femera::proc::Fmpi()));
#endif
#ifdef FMR_HAS_OMP
  fmr_main->add_task (std::make_shared<femera::proc::Fomp> (femera::proc::Fomp()));
#endif
  fmr_main->add_task (std::make_shared<femera::proc::Fcpu> (femera::proc::Fcpu()));
  fmr_main->init (&argc,argv);
  return femera::test:: early_main (&argc, argv);
}
