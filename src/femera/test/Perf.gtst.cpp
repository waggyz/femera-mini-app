#include "../femera.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return mini->exit (mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}
namespace femera { namespace test { namespace perf {
  inline
  double test_proc_id_speed (const fmr::Perf_int n=100000l) {
    fmr::Local_int ans1=0, ans2=0;
    mini->test->time.add_idle_time_now ();
    for (fmr::Perf_int i=0; i<n; ++i) {
      ans1 += mini->proc->get_team_n ();// just returns a member variable
    }
    const auto base_time = mini->test->time.add_busy_time_now ();// - start;
    mini->test->time.add_idle_time_now ();
    for (fmr::Perf_int i=0; i<n; ++i) {
      ans2 += mini->proc->get_proc_id ();// descends the processing stack
    }
    const auto pids_time = mini->test->time.add_busy_time_now ();// - start;
    const auto ratio = double (pids_time / base_time);
    const auto base_str
      = fmr::form::si_unit (double (base_time) / double (n),"s");
    const auto pids_str
      = fmr::form::si_unit (double (pids_time) / double (n),"s");
    if (ans2 >= ans1) {// should always be true
      mini->data->send (fmr::log, "perf", "gtst", "proc",
        "%s /%s (pid/ref) = %.0fx slower each call",
        pids_str.c_str(), base_str.c_str(), ratio - 1.0);
    }
    return ratio;
  }
  TEST( TestPerf, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
FMR_WARN_INLINE_OFF
  TEST( TestProcIDPerf, PidsIsSlower ){
    EXPECT_GT( test_proc_id_speed (), 1.0 );
  }
FMR_WARN_INLINE_ON

} } }//end femera::test::perf:: namespace

