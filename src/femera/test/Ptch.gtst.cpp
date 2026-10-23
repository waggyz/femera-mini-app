#include "../femera.hpp"
#include "Mesh.hpp"
#include "Ptch.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc, argv));
}

TEST (Ptch, TaskIdentityAndLifecycle) {
  auto task = femera::Test<femera::test::Ptch>::new_task (mini->get_core ());
  EXPECT_EQ (task->get_task_type (),
    femera::task_cast (femera::Task_type::Ptch));
  EXPECT_EQ (task->get_abrv (), "ptch");
  EXPECT_EQ (task->init (nullptr, nullptr), 0);
  EXPECT_TRUE (task->did_init ());
  EXPECT_EQ (task->exit (), 0);
  EXPECT_FALSE (task->did_init ());
}

TEST (Ptch, UsesSharedMeshFixtureContract) {
  const auto mesh = femera::test::reference_tet4 ();
  EXPECT_EQ (mesh.element_count, 1);
  EXPECT_EQ (mesh.nodes_per_element, 4);
  EXPECT_TRUE (mesh.interior_nodes.empty ());
}
