#include "elastic-linear.hpp"

#include <gtest/gtest.h>

TEST(ElasticGeneral, TrivialTest) {
  EXPECT_EQ(1,1);
}

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
/*
fmr::Exit_int main (int argc, char** argv) {
const auto mini_ptr = std::move (fmr::new_jobs (& argc, argv));
auto& mini = *mini_ptr;
//FMR_WARN_INLINE_OFF
  return mini.exit (mini.init (&argc, argv));
//FMR_WARN_INLINE_ON
}
*/
