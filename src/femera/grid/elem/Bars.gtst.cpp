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
TEST( GridElemBars, ShapeFunctionsAndGradients ){
  const Bars bars;// Default linear 2-node element
  const double xi = 0.5;// integration point for Gauss Legendre first order quadrature
  double N_storage[2];
  double dN_storage[2 * 3];// Default 3D space
  double* N = N_storage;
  double* dN = dN_storage;
  bars.poly_func_2 (N, &xi);
  bars.poly_grad_2 (dN, &xi);
  ASSERT_NE( N, nullptr );
  ASSERT_NE( dN, nullptr );
  EXPECT_DOUBLE_EQ( N[0], 0.5 );
  EXPECT_DOUBLE_EQ( N[1], 0.5 );
  EXPECT_DOUBLE_EQ( dN[0], -1.0 );
  EXPECT_DOUBLE_EQ( dN[1], 1.0 );
  EXPECT_DOUBLE_EQ( dN[2], 0.0 );
  EXPECT_DOUBLE_EQ( dN[3], 0.0 );
  EXPECT_DOUBLE_EQ( dN[4], 0.0 );
  EXPECT_DOUBLE_EQ( dN[5], 0.0 );
}
TEST( GridElemBars, JacobianCalculation ){
  const Bars bars;
  const double xi = 0.5;
  double dN_storage[2 * 3];
  double* dN = dN_storage;
  double dNT_storage[2 * 3];
  double* dNT = dNT_storage;
  bars.poly_grad_2 (dN, &xi);
  bars.poly_grat_2 (dNT, &xi);
  fmr::Phys_float jacs_naos_storage[3 * 3];
  fmr::Phys_float* jacs_naos = jacs_naos_storage;
  fmr::Phys_float jacs_nsoa_storage[3 * 3];
  fmr::Phys_float* jacs_nsoa = jacs_nsoa_storage;
  const double* node_coor = bars.get_vert_coor ();// naos
  const double* coor_node = bars.get_coor_vert ();// nsoa
  //
  bars.calc_jacs_naos_ref (jacs_naos, node_coor, dNT, 1u, 3u);
  bars.calc_jacs_nsoa_ref (jacs_nsoa, coor_node, dNT, 1u, 3u);
  //
  // A finite element with nodes at natural element locations should have an
  // identity Jacobian.
  EXPECT_DOUBLE_EQ( jacs_naos[0], 1.0 );
  EXPECT_DOUBLE_EQ( jacs_naos[1], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_naos[2], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_naos[3], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_naos[4], 1.0 );
  EXPECT_DOUBLE_EQ( jacs_naos[5], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_naos[6], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_naos[7], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_naos[8], 1.0 );
  //
  EXPECT_DOUBLE_EQ( jacs_nsoa[0], jacs_naos[0] );
  EXPECT_DOUBLE_EQ( jacs_nsoa[1], jacs_naos[1] );
  EXPECT_DOUBLE_EQ( jacs_nsoa[2], jacs_naos[2] );
  EXPECT_DOUBLE_EQ( jacs_nsoa[3], jacs_naos[3] );
  EXPECT_DOUBLE_EQ( jacs_nsoa[4], jacs_naos[4] );
  EXPECT_DOUBLE_EQ( jacs_nsoa[5], jacs_naos[5] );
  EXPECT_DOUBLE_EQ( jacs_nsoa[6], jacs_naos[6] );
  EXPECT_DOUBLE_EQ( jacs_nsoa[7], jacs_naos[7] );
  EXPECT_DOUBLE_EQ( jacs_nsoa[8], jacs_naos[8] );
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
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[Bars::tile_node_n] = {0, 0};

  EXPECT_TRUE( get_tile_conn (conn, 0, part_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
  EXPECT_TRUE( get_tile_conn (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 4 );
  EXPECT_FALSE( get_tile_conn (conn, 4, part_size) );
}

TEST( GridElemBars, TileConnectivityPeriodic ){
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[Bars::tile_node_n] = {0, 0};

  EXPECT_TRUE( get_tile_conn_prdc (conn, 0, part_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
  EXPECT_TRUE( get_tile_conn_prdc (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 0 );
  EXPECT_FALSE( get_tile_conn_prdc (conn, 4, part_size) );
}

TEST( GridElemBars, PartitionedTileConnectivity ){
  const fmr::Local_int part_size[1] = {3};
  fmr::Local_int conn[Bars::tile_node_n] = {0, 0};

  EXPECT_FALSE( get_tile_conn (conn, 4, part_size) );
  EXPECT_FALSE( get_tile_conn (conn, 5, part_size) );
}

TEST( GridElemBars, TileNodeCoordinates ){
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int node_n = 3;
  fmr::Geom_float coor[3];

  EXPECT_TRUE( get_tile_node_coor(coor, 0, node_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );
  EXPECT_TRUE( get_tile_node_coor(coor, 2, node_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( coor[2], 2.0 );
  EXPECT_FALSE( get_tile_node_coor(coor, 3, node_n, elem_size, part_orig) );
}
TEST( GridElemBars, MoreTileNodeCoordinates ){
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0,2.0, 3.0};
  const fmr::Local_int node_n = 3;
  fmr::Geom_float coor[3];

  EXPECT_TRUE( get_tile_node_coor(coor, 0, node_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );
  EXPECT_TRUE( get_tile_node_coor(coor, 2, node_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 4.0 );
  EXPECT_DOUBLE_EQ( coor[2], 6.0 );
  EXPECT_FALSE( get_tile_node_coor(coor, 3, node_n, elem_size, part_orig) );
}

TEST( GridElemBars, TileElementVertexCoordinatesAOS ){
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[Bars::tile_elem_n * 2 * 3];// 2 vertices per element, 3 coordinates each

  EXPECT_TRUE( get_tile_elem_vert_coor(vert_coor, 0, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 1.0 );
  EXPECT_TRUE( get_tile_elem_vert_coor(vert_coor, 1, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 2.0 );
  EXPECT_FALSE( get_tile_elem_vert_coor(vert_coor, 2, elem_n, elem_size, part_orig) );
}

TEST( GridElemBars, TileElementVertexCoordinatesSOA ){
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[Bars::tile_elem_n * 2 * 3];// 2 vertices per element, 3 coordinates each
  EXPECT_TRUE( get_tile_elem_coor_vert(vert_coor, 0, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 1.0 );
  EXPECT_TRUE( get_tile_elem_coor_vert(vert_coor, 1, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 2.0 );
  EXPECT_FALSE( get_tile_elem_coor_vert(vert_coor, 2, elem_n, elem_size, part_orig) );
}

} } } // end namespace femera::grid::elem
