#include "../femera.hpp"
#include "Mesh.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc, argv));
}

TEST (TestMesh, ReferenceTet4Contract) {
  const auto mesh = femera::test::reference_tet4 ();
  EXPECT_EQ (mesh.spatial_dimension, 3);
  EXPECT_EQ (mesh.node_count, 4);
  EXPECT_EQ (mesh.element_count, 1);
  EXPECT_EQ (mesh.nodes_per_element, 4);
  ASSERT_EQ (mesh.coordinates.size (), 12u);
  ASSERT_EQ (mesh.connectivity.size (), 4u);
  EXPECT_EQ (mesh.connectivity, (std::vector<fmr::Local_int>{0, 1, 2, 3}));
  EXPECT_EQ (mesh.boundary_nodes,
    (std::vector<fmr::Local_int>{0, 1, 2, 3}));
  EXPECT_TRUE (mesh.interior_nodes.empty ());
  EXPECT_EQ (mesh.component_major_coordinates (),
    (std::vector<fmr::Geom_float>{
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0}));
}
