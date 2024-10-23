#include "../femera.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

fmr::Local_int sims_task_n = 0;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (& argc, argv);
  //
  auto sim = mini.get_task (femera::Task_type::Sims);
  sims_task_n = sim->get_task_n ();
  //
  FMR_PRAGMA_OMP(omp barrier)
  return mini.exit ();
}
TEST( TaskSims, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
TEST( TaskSims, RunsGT0 ){
  EXPECT_GT( sims_task_n, 0 );
}
#ifdef FMR_RUNS_LOCAL
TEST( TaskSims, RunsGT1 ){
  EXPECT_GT( sims_task_n, 1 );
}
#endif

