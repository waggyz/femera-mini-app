#include "../femera.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
#pragma GCC diagnostic ignored "-Winline"
  return mini->exit (mini->init (&argc,argv));
#pragma GCC diagnostic warning "-Winline"
}
TEST( Gtst, TaskName ){
  EXPECT_EQ( mini->test->get_task (0)->get_name (), "GoogleTest" );
}
