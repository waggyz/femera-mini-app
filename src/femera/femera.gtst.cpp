#include "femera.hpp"

#include "gtest/gtest.h"

auto mini = fmr::new_sims ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
TEST(NewSims, AbrvIsJobs) {
  EXPECT_EQ( mini->abrv, "jobs");
}
