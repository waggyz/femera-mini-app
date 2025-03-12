#include "femera.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return mini->exit (mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}
TEST(NewTask, AbrvIsJobs) {
  EXPECT_EQ( mini->get_abrv (), "jobs");
  EXPECT_EQ( mini->get_version ().substr(0,6), "Femera");
  EXPECT_EQ( mini->get_version (), MAKESTR(FMR_VERSION));
}
