#include "../femera.hpp"

#include <gtest/gtest.h>

fmr::Exit_int main (int argc, char** argv) {
  auto  mini_ptr = fmr::new_jobs (& argc, argv);
  auto& mini = *mini_ptr;
  //
  FMR_WARN_INLINE_OFF
    return mini.exit ();
  FMR_WARN_INLINE_ON
}
TEST( PhysMtrlElas, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

