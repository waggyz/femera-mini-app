#include "../femera.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
TEST( TestBeds, TaskName ){
  EXPECT_EQ( mini->test->get_abrv (), "beds" );
}
