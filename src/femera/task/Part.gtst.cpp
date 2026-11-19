#include "../femera.hpp"

#include <gtest/gtest.h>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (& argc, argv);
  //
  //
  FMR_PRAGMA_OMP(omp barrier)
  return mini->exit ();
}
TEST( TaskPart, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
