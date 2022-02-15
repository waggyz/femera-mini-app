#include "../femera.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <vector>

auto mini = femera::new_sims ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
TEST( SelfTestProc, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
namespace femera { namespace test { namespace self {
#if 1
inline
std::vector<fmr::Local_int> get_local_proc_ids () {//TODO all_gathered_proc_ids()
//  mini->test->time.add_idle_time_now ();
  const fmr::Local_int  all_n = mini->proc->all_proc_n ();
  const fmr::Local_int each_n = 2;//FIXME
  const fmr::Local_int  mod_n = all_n / each_n;
  std::vector<fmr::Local_int> pids (each_n);
  FMR_PRAGMA_OMP(omp parallel) {
    const auto id = mini->proc->get_proc_id ();
    if (id % mod_n < each_n) {
      FMR_PRAGMA_OMP(omp atomic write)
      pids [id % mod_n] = id;
    }
    else {}
    FMR_PRAGMA_OMP(omp barrier)
  }
//  mini->test->time.add_busy_time_now ();
  return pids;
}
inline
std::vector<fmr::Local_int> get_local_numa_ixs () {
//  mini->test->time.add_idle_time_now ();
  const fmr::Local_int  all_n = mini->proc->all_proc_n ();
  const fmr::Local_int each_n = 2;//FIXME
  const fmr::Local_int  mod_n = all_n / each_n;
  std::vector<fmr::Local_int> numas (each_n);
  FMR_PRAGMA_OMP(omp parallel) {
    const auto id = fmr::proc::get_node_numa_ix ();
    if (id % mod_n < each_n) {
      FMR_PRAGMA_OMP(omp atomic write)
      numas [id % mod_n] = id;
    }
    else {}
    FMR_PRAGMA_OMP(omp barrier)
  }
//  mini->test->time.add_busy_time_now ();
  return numas;
}
using ::testing::ElementsAreArray;
using ::testing::Each;

TEST( SelfTestProc, LocalProcIDs ) {//TODO only main MPI process reported
  EXPECT_THAT( get_local_proc_ids (),
    ElementsAreArray( std::vector<fmr::Local_int> ({0,1})));
}
TEST( SelfTestProc, LocalNumasAllSame ) {//TODO only main MPI process reported
  EXPECT_THAT( get_local_numa_ixs (), Each( get_local_numa_ixs ()[0]) );
}
#endif
} } }//end femera::test::self:: namespace
