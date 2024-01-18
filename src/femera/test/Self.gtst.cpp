#include "../femera.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>

auto mini = fmr::new_jobs ();

namespace femera { namespace test { namespace self {
#if 1
  inline
  std::vector<fmr::Local_int> get_local_proc_ids () {//TODO all_gathered_proc_ids()
    const fmr::Local_int  all_n = mini->proc->all_proc_n ();
    const fmr::Local_int each_n = 2;// TDD
    const fmr::Local_int  mod_n = all_n / each_n;
    std::vector<fmr::Local_int> pids (each_n);
    FMR_PRAGMA_OMP(omp parallel) {
      const auto id = mini->proc->get_proc_id ();
      if (id % mod_n < each_n) {
        FMR_PRAGMA_OMP(omp atomic write)
        pids [id % mod_n] = id;
      mini->data->send (fmr::log, "self", "gtst", "proc",
        "%4u proc[%u %% %u=%u]=%u%s", mini->proc->get_proc_id (), id, mod_n,
        id % mod_n, pids [id % mod_n], mini->proc->is_main() ? " *main" : "");
      }
      else {}
      FMR_PRAGMA_OMP(omp barrier)
    }
    return pids;
  }
  inline
  std::vector<fmr::Local_int> get_local_numa_ixs () {
    const fmr::Local_int  all_n = mini->proc->all_proc_n ();
    const fmr::Local_int each_n = 2;// TDD
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
    return numas;
  }
  using ::testing::ElementsAreArray;
  using ::testing::Each;
  
  TEST( SelfTestProc, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
FMR_WARN_INLINE_OFF
  TEST( SelfTestProc, LocalProcIDs ){
    EXPECT_THAT( get_local_proc_ids (),
      ElementsAreArray( std::vector<fmr::Local_int>// ({0,1})));
        ({mini->proc->get_proc_id (), mini->proc->get_proc_id () + 1})));
  }
FMR_WARN_INLINE_ON
  TEST( SelfTestProc, LocalNumasAllSame ){
    EXPECT_THAT( get_local_numa_ixs (), Each( get_local_numa_ixs ()[0]) );
  }
#endif
} } }//end femera::test::self:: namespace

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return mini->exit (mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}

