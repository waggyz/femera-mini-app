#include "../femera.hpp"
#include "FE1e.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc, argv));
}

TEST (FE1e, TaskIdentityAndLifecycle) {
  auto task = femera::Test<femera::test::FE1e>::new_task (mini->get_core ());
  EXPECT_EQ (task->get_task_type (),
    femera::task_cast (femera::Task_type::FE1e));
  EXPECT_EQ (task->get_abrv (), "fe1e");
  EXPECT_EQ (task->init (nullptr, nullptr), 0);
  EXPECT_TRUE (task->did_init ());
  EXPECT_EQ (task->exit (), 0);
  EXPECT_FALSE (task->did_init ());
}
