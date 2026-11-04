#include "../../femera.hpp"
#include "../Elem.test.hpp"
#include "Bars.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}
namespace femera { namespace grid { namespace elem {
TEST( GridElemBars, ElementTopology ){
  const Bars bars;
  EXPECT_EQ( static_cast<fmr::Local_int>(Bars::sims_d), 3u );
  EXPECT_EQ( static_cast<fmr::Local_int>(Bars::elem_d), 1u );
  EXPECT_EQ( bars.get_sims_d (), 3u );
  EXPECT_EQ( bars.get_elem_d (), 1u );
  EXPECT_EQ( bars.get_vert_n (), 2u );
  EXPECT_EQ( bars.get_edge_n (), 1u );
  EXPECT_EQ( bars.get_tris_n (), 0u );
  EXPECT_EQ( bars.get_quad_n (), 0u );
  EXPECT_EQ( bars.get_vols_n (), 0u );
  EXPECT_EQ( bars.get_face_n (), 0u );
  EXPECT_EQ( bars.get_node_n (), 2u );
  EXPECT_DOUBLE_EQ( bars.get_edge_l (), 1.0 );
  EXPECT_DOUBLE_EQ( bars.get_face_a (), 0.0 );
  EXPECT_DOUBLE_EQ( bars.get_elem_v (), 0.0 );
}

TEST( GridElemBars, ElementOrderNodeCounts ){
  const Elem<Bars> linear;
  const Elem<Bars, 2> quadratic;
  const Elem<Bars, 3> cubic;
  EXPECT_EQ( linear.get_elem_p (), 1 );
  EXPECT_EQ( quadratic.get_elem_p (), 2 );
  EXPECT_EQ( cubic.get_elem_p (), 3 );
  EXPECT_EQ( linear.get_node_n (), 2 );
  EXPECT_EQ( quadratic.get_node_n (), 3 );
  EXPECT_EQ( cubic.get_node_n (), 4 );
  EXPECT_EQ( cubic.poly_func_size (), 4 );
  EXPECT_EQ( cubic.poly_grad_size (), 12 );
}

TEST( GridElemBars, VertexConnectivityAndCoordinates ){
  const Bars bars;
  ASSERT_NE( bars.get_bars_conn (), nullptr );
  constexpr fmr::Local_int vert0 = Bars::vert_conn[0];
  constexpr fmr::Local_int vert1 = Bars::vert_conn[1];
  EXPECT_EQ( vert0, 0u );
  EXPECT_EQ( vert1, 1u );
  EXPECT_EQ( bars.get_bars_conn ()[0], 0 );
  EXPECT_EQ( bars.get_bars_conn ()[1], 1 );

  const double* vert_coor = bars.get_vert_coor ();
  const double* coor_vert = bars.get_coor_vert ();
  ASSERT_NE( vert_coor, nullptr );
  ASSERT_NE( coor_vert, nullptr );
  EXPECT_DOUBLE_EQ( vert_coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 0.0 );
  EXPECT_DOUBLE_EQ( coor_vert[0], 0.0 );
  EXPECT_DOUBLE_EQ( coor_vert[1], 1.0 );
  EXPECT_DOUBLE_EQ( coor_vert[2], 0.0 );
  EXPECT_DOUBLE_EQ( coor_vert[3], 0.0 );
  EXPECT_DOUBLE_EQ( coor_vert[4], 0.0 );
  EXPECT_DOUBLE_EQ( coor_vert[5], 0.0 );

  for (fmr::Local_int vertex = 0; vertex < Bars::vert_n; ++vertex) {
    for (fmr::Local_int dim = 0; dim < Bars::sims_d; ++dim) {
      EXPECT_DOUBLE_EQ(
        vert_coor[vertex * Bars::sims_d + dim],
        coor_vert[dim * Bars::vert_n + vertex]);
    }
  }
}

TEST( GridElemBars, TileMetadata ){
  const Bars bars;
  EXPECT_EQ( static_cast<fmr::Local_int>(Bars::tile_elem_n), 1u );
  EXPECT_EQ( static_cast<fmr::Local_int>(Bars::tile_node_n),
    static_cast<fmr::Local_int>(Bars::vert_n) );
  EXPECT_EQ( bars.get_tile_elem_n (), 1u );
  EXPECT_EQ( bars.get_tile_node_n (), 2u );
  constexpr fmr::Local_int tile0 = Bars::tile_conn[0];
  constexpr fmr::Local_int tile1 = Bars::tile_conn[1];
  EXPECT_EQ( tile0, 0u );
  EXPECT_EQ( tile1, 1u );
}

TEST( GridElemBars, TileConnectivity ){
  const fmr::Local_int grid_size[1] = {4};
  fmr::Local_int conn[Bars::tile_node_n] = {0, 0};

  EXPECT_TRUE( get_tile_conn (conn, 0, grid_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
  EXPECT_TRUE( get_tile_conn (conn, 3, grid_size) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 0 );
  EXPECT_FALSE( get_tile_conn (conn, 4, grid_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
}

TEST( GridElemBars, PartitionedTileConnectivity ){
  const fmr::Local_int grid_size[1] = {5};
  const fmr::Local_int part_size[1] = {3};
  fmr::Local_int conn[Bars::tile_node_n] = {0, 0};

  EXPECT_TRUE( get_tile_conn (conn, 4, grid_size, part_size) );
  EXPECT_EQ( conn[0], 1 );
  EXPECT_EQ( conn[1], 0 );
  EXPECT_FALSE( get_tile_conn (conn, 5, grid_size, part_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
}
} } } // end namespace femera::grid::elem
