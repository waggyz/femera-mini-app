#include "../femera.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (& argc, argv);
  //
  //
  FMR_PRAGMA_OMP(omp barrier)
  return mini.exit ();
}
TEST( TaskPart, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
