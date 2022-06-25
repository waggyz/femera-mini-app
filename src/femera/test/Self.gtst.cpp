#include "../femera.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
#pragma GCC diagnostic ignored "-Winline"
  return mini->exit (mini->init (&argc,argv));
#pragma GCC diagnostic warning "-Winline"
}
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
        const auto name = femera::form::text_line (40,"%4s %4s %4s",
          mini->test->get_base_abrv().c_str(),
            mini->test->get_abrv().c_str(),"proc");
        const std::string text = std::to_string(id);
#pragma GCC diagnostic ignored "-Winline"
        mini->data->name_line (mini->data->fmrout, name,
          "%4u proc[%u %% %u=%u]=%u%s", mini->proc->get_proc_id (), id, mod_n,
          id % mod_n, pids [id % mod_n], mini->proc->is_main() ? " *main" : "");
#pragma GCC diagnostic warning "-Winline"
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
#pragma GCC diagnostic ignored "-Winline"
  TEST( SelfTestProc, LocalProcIDs ){
    EXPECT_THAT( get_local_proc_ids (),
      ElementsAreArray( std::vector<fmr::Local_int>// ({0,1})));
        ({mini->proc->get_proc_id (), mini->proc->get_proc_id () + 1})));
  }
#pragma GCC diagnostic warning "-Winline"
  TEST( SelfTestProc, LocalNumasAllSame ){
    EXPECT_THAT( get_local_numa_ixs (), Each( get_local_numa_ixs ()[0]) );
  }
#endif
} } }//end femera::test::self:: namespace
