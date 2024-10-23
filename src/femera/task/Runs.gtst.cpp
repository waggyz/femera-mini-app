#include "../femera.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

std::vector<fmr::Local_int> run_own;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (& argc, argv);
  //
  auto sim = mini.get_task (femera::Task_type::Sims);
  fmr::Local_int o = 1;
#ifdef FMR_RUNS_LOCAL
  o =  fmr::Local_int (::omp_get_max_threads ());
#endif
  run_own.resize (o);
  FMR_PRAGMA_OMP(omp parallel for schedule(static) num_threads(o))
  for (fmr::Local_int i=0; i<o; ++i) {
    auto run = sim->get_task (femera::Task_type::Runs, i);
    run_own [fmr::Local_int (::omp_get_thread_num ())]
      = run->get_thrd_ownr ();
  }
  // Don't exit (and run tests) until all OpenMP threads complete.
  FMR_PRAGMA_OMP(omp barrier)
  return mini.exit ();
}
TEST( TaskRuns, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
TEST( TaskRuns, ThrdOwnr0 ){
  EXPECT_EQ( run_own.at(0), 0 );
}
#ifdef FMR_RUNS_LOCAL
TEST( TaskRuns, ThrdOwnr1 ){
  EXPECT_EQ( run_own.at(1), 1 );
}
#endif
