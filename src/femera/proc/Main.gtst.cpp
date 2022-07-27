#include "Main.hpp"

#include <gtest/gtest.h>

namespace femera { namespace test {
FMR_WARN_INLINE_OFF
  auto main_test = FMR_MAKE_SMART(femera::proc::Main) (femera::proc::Main());
FMR_WARN_INLINE_ON
  inline
  fmr::Local_int proc_id_sum () {fmr::Local_int sum = 0;
    FMR_PRAGMA_OMP(omp parallel reduction( +:sum)) {
      const auto id = main_test->get_proc_id ();
      sum += id;
      printf ("main test proc_id%4u/%4u\n",id, main_test->all_proc_n ());
    }
    return sum;
  }
  TEST( Main, TaskName ){
    EXPECT_EQ( main_test->get_abrv (), "main");
    EXPECT_EQ( main_test->get_task
      (femera::Work::Task_path_t())->get_abrv (), "main");
  }
  TEST( Main, DidInit ){
    EXPECT_EQ( main_test->get_task_n(), uint(1) );
#if 0
    EXPECT_EQ( main_test->get_task(femera::Task_type::Root)->get_abrv (), "root");
#endif
    EXPECT_EQ( main_test->get_task(femera::Task_type::Node)->get_abrv (), "node");
#ifdef FMR_HAS_MPI
    EXPECT_EQ( main_test->get_task(femera::Task_type::Fmpi)->get_abrv (), "mpi");
#endif
#ifdef FMR_HAS_OPENMP
    EXPECT_GT( proc_id_sum (), 0);
    EXPECT_GT( main_test->get_race_n (), 1);
#endif
  }
} }//end femera::test:: namespcae
int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  femera::test::main_test->init (& argc, argv);
  return femera::test:: early_main (& argc, argv);
FMR_WARN_INLINE_ON
}
