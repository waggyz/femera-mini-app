#include "../../femera.hpp"
#include "Ipn1.hpp"

#include <gtest/gtest.h>

#include <type_traits>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}
TEST( GridTileIpn1, TrivialTest ){
  EXPECT_TRUE( true );
}
namespace femera { namespace grid { namespace tile {

TEST( GridTileIpn1, IsStateless ){
  EXPECT_TRUE( (std::is_empty<Ipn1>::value) );
  EXPECT_TRUE( (std::is_empty<Tile<Ipn1>>::value) );
}

TEST( TileElemIpn1, ElementOrderNodeCounts ){
  EXPECT_EQ( Tile<Ipn1>::get_tile_vert_n (), 2 );
  EXPECT_EQ( Tile<Ipn1>::get_tile_elem_node_n (), 2 );

  constexpr Tile<Ipn1> linear;
  constexpr Tile<Ipn1, 2> quadratic;
  constexpr Tile<Ipn1, 3> cubic;

  EXPECT_EQ( linear.get_tile_vert_n (), 2 );
  EXPECT_EQ( quadratic.get_tile_vert_n (), 2 );
  EXPECT_EQ( cubic.get_tile_vert_n (), 2 );

  EXPECT_EQ( linear.get_tile_elem_node_n (), 2 );
  EXPECT_EQ( quadratic.get_tile_elem_node_n (), 3 );
  EXPECT_EQ( cubic.get_tile_elem_node_n (), 4 );
}
TEST( GridTileIpn1, TileMetadata ){
  constexpr Ipn1 ipn1;
  EXPECT_EQ( static_cast<fmr::Local_int>(ipn1.get_tile_elem_n ()), 1u );
  EXPECT_EQ( static_cast<fmr::Local_int>(ipn1.get_tile_elem_node_n ()), 2u );
#if 0
  EXPECT_EQ( static_cast<fmr::Local_int>(ipn1.get_tile_elem_node_n ()),
    static_cast<fmr::Local_int>(ipn1.get_vert_n ()) );// P1 only
  EXPECT_EQ( static_cast<fmr::Local_int>(ipn1.get_tile_vert_n ()),
    static_cast<fmr::Local_int>(ipn1.get_vert_n ()) );
#endif
  EXPECT_EQ( static_cast<fmr::Local_int>(ipn1.get_tile_vert_n ()), 2u );
  EXPECT_EQ( ipn1.get_tile_elem_n (), 1u );
  constexpr fmr::Local_int tile0 = 0;
  constexpr fmr::Local_int tile1 = 1;
  EXPECT_EQ( tile0, 0u );
  EXPECT_EQ( tile1, 1u );
}
TEST( GridTileIpn1, TileMetadataWithoutInstance ){
  EXPECT_EQ( Tile<Ipn1>::get_tile_vert_n (), 2 );
  EXPECT_EQ( Tile<Ipn1>::get_tile_elem_node_n (), 2 );
  EXPECT_EQ( Tile<Ipn1>::get_tile_elem_n (), 1 );
}

struct TileDispatchTestShape {
  static constexpr bool calc_tile_vert_conn (fmr::Local_int* conn,
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Local_int p) noexcept {
    conn[0] = tile_i + p;
    return tile_i < part_size[0];
  }
};
constexpr bool tile_dispatch_constexpr_test () {
  const Tile<TileDispatchTestShape, 3> shape;
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[1] = {0};
  return shape.get_tile_vert_conn (conn, 2, part_size) && conn[0] == 5;
}
static_assert (tile_dispatch_constexpr_test (), "Tile dispatch must be constexpr");

// Tiling tests ===============================================================

TEST( GridTileIpn1, TilePartElemCount ){
  //get_tile_part_elem_n
  constexpr Ipn1 ipn1; // only [0] is used for Ipn1.
  const fmr::Local_int part_elem_dims_a[3] = {2, 0, 0};
  const fmr::Local_int part_elem_dims_b[3] = {2, 3, 4};
  const fmr::Local_int part_elem_dims_c[3] = {0, 0, 0};
  const fmr::Local_int part_elem_dims_d[3] = {0, 3, 4};
  //NOTE only [0] is used for 1D elements like Ipn1
  EXPECT_EQ( ipn1.get_tile_part_elem_n (part_elem_dims_a), 2);
  EXPECT_EQ( ipn1.get_tile_part_elem_n (part_elem_dims_b), 2);
  EXPECT_EQ( ipn1.get_tile_part_elem_n (part_elem_dims_c), 0);
  EXPECT_EQ( ipn1.get_tile_part_elem_n (part_elem_dims_d), 0);

  EXPECT_EQ( Tile<Ipn1>::get_tile_part_elem_n (part_elem_dims_a), 2);
}

TEST( GridTileIpn1, TileVertexConnectivity ){
  constexpr Ipn1 ipn1_1;
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[ipn1_1.get_tile_vert_n ()] = {0, 0};

  EXPECT_TRUE( ipn1_1.get_tile_vert_conn (conn, 0u, part_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
  EXPECT_TRUE( ipn1_1.get_tile_vert_conn (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 4 );
  EXPECT_FALSE( ipn1_1.get_tile_vert_conn (conn, 4, part_size) );

  EXPECT_TRUE( Tile<Ipn1>::get_tile_vert_conn (conn, 0u, part_size) );
}

TEST( GridTileIpn1, TileVertexConnectivityPeriodic ){
  constexpr Ipn1 ipn1_1;// linear
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[ipn1_1.get_tile_vert_n ()] = {0, 0};

  EXPECT_TRUE( ipn1_1.get_tile_vert_conn_prdc (conn, 0, part_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
  EXPECT_TRUE( ipn1_1.get_tile_vert_conn_prdc (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 0 );
  EXPECT_FALSE( ipn1_1.get_tile_vert_conn_prdc (conn, 4, part_size) );
}

TEST( GridTileIpn1, HigherOrderTileVertexConnectivity ){
  constexpr Tile<Ipn1, 2> quadratic;
  constexpr Tile<Ipn1, 3> cubic;
  const fmr::Local_int part_size[1] = {4};
  const fmr::Local_int empty_part[1] = {0};
  fmr::Local_int conn[2] = {0, 0};

  EXPECT_TRUE( quadratic.get_tile_vert_conn (conn, 2, part_size) );
  EXPECT_EQ( conn[0], 4 );
  EXPECT_EQ( conn[1], 6 );
  EXPECT_TRUE( cubic.get_tile_vert_conn (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 9 );
  EXPECT_EQ( conn[1], 12 );
  EXPECT_TRUE( quadratic.get_tile_vert_conn_prdc (conn, 2, part_size) );
  EXPECT_EQ( conn[0], 4 );
  EXPECT_EQ( conn[1], 6 );
  EXPECT_TRUE( cubic.get_tile_vert_conn_prdc (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 9 );
  EXPECT_EQ( conn[1], 0 );
  EXPECT_TRUE( quadratic.get_tile_vert_conn (conn, 1, part_size, 3) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 6 );
  EXPECT_FALSE( quadratic.get_tile_vert_conn_prdc (conn, 0, empty_part) );

  EXPECT_TRUE( (Tile<Ipn1, 2>::get_tile_vert_conn (conn, 2, part_size)) );
}

TEST( GridTileIpn1, TileNodeConnectivity ){
  constexpr Tile<Ipn1> linear;
  constexpr Tile<Ipn1, 2> quadratic;
  constexpr Tile<Ipn1, 3> cubic;
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int linear_conn[2] = {0, 0};
  fmr::Local_int quadratic_conn[3] = {0, 0, 0};
  fmr::Local_int cubic_conn[4] = {0, 0, 0, 0};

  EXPECT_TRUE( linear.get_tile_node_conn (linear_conn, 2, part_size) );
  EXPECT_EQ( linear_conn[0], 2 );
  EXPECT_EQ( linear_conn[1], 3 );

  EXPECT_TRUE( quadratic.get_tile_node_conn (quadratic_conn, 2, part_size) );
  EXPECT_EQ( quadratic_conn[0], 4 );
  EXPECT_EQ( quadratic_conn[1], 6 );
  EXPECT_EQ( quadratic_conn[2], 5 );

  EXPECT_TRUE( cubic.get_tile_node_conn (cubic_conn, 2, part_size) );
  EXPECT_EQ( cubic_conn[0], 6 );
  EXPECT_EQ( cubic_conn[1], 9 );
  EXPECT_EQ( cubic_conn[2], 7 );
  EXPECT_EQ( cubic_conn[3], 8 );

  EXPECT_FALSE( cubic.get_tile_node_conn (cubic_conn, 4, part_size) );

  EXPECT_TRUE( (Tile<Ipn1>::get_tile_node_conn (linear_conn, 2, part_size)) );
}

TEST( GridTileIpn1, PartitionedTileVertexConnectivity ){
  constexpr Ipn1 ipn1;
  const fmr::Local_int part_size[1] = {3};
  fmr::Local_int conn[ipn1.get_tile_vert_n ()] = {0, 0};

  EXPECT_FALSE( ipn1.get_tile_vert_conn (conn, 4, part_size) );
  EXPECT_FALSE( ipn1.get_tile_vert_conn (conn, 5, part_size) );
}

TEST( GridTileIpn1, TileVertexCoordinates ){
  Ipn1 ipn1;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  //const fmr::Local_int part_node_dims[3] = {3,0,0};
  const fmr::Local_int vert_n = 3;
  fmr::Geom_float coor[3];

  EXPECT_TRUE( ipn1.get_tile_vert_coor(coor, 0, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );
  EXPECT_TRUE( ipn1.get_tile_vert_coor(coor, 2, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( coor[2], 2.0 );
  EXPECT_FALSE( ipn1.get_tile_vert_coor(coor, 3, vert_n, elem_size, part_orig) );
}

TEST( GridTileIpn1, MoreTileVertexCoordinates ){
  Ipn1 ipn1;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0,2.0, 3.0};
  const fmr::Local_int vert_n = 3;
  fmr::Geom_float coor[3];

  EXPECT_TRUE( ipn1.get_tile_vert_coor(coor, 0, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );
  EXPECT_TRUE( ipn1.get_tile_vert_coor(coor, 2, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 4.0 );
  EXPECT_DOUBLE_EQ( coor[2], 6.0 );
  EXPECT_FALSE( ipn1.get_tile_vert_coor(coor, 3, vert_n, elem_size, part_orig) );
}
TEST( GridTileIpn1, TileNodeCountAndCoordinates ){
  constexpr Tile<Ipn1> ipn1_p1;// linear default, 4 nodes in 3 elems
  constexpr Tile<Ipn1, 2> ipn1_p2;// 7 nodes in 3 elems
  constexpr Tile<Ipn1, 3> ipn1_p3;// 10 nodes in 3 elems
  const fmr::Geom_float part_orig[3] = {0.0, 1.0, 2.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int part_elem_dims[3] = {3,0,0};
  fmr::Geom_float coor[3];
  //
  EXPECT_EQ( ipn1_p1.get_tile_part_node_n(part_elem_dims), 4 );
  EXPECT_EQ( ipn1_p2.get_tile_part_node_n(part_elem_dims), 7 );
  EXPECT_EQ( ipn1_p3.get_tile_part_node_n(part_elem_dims), 10 );
  //
  EXPECT_TRUE( ipn1_p1.get_tile_part_node_coor(coor, 0, part_elem_dims, elem_size) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );// default origin
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );

  EXPECT_TRUE( ipn1_p1.get_tile_part_node_coor(coor, 2, part_elem_dims, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 3.0 );
  EXPECT_DOUBLE_EQ( coor[2], 4.0 );
  EXPECT_TRUE( ipn1_p1.get_tile_part_node_coor(coor, 3, part_elem_dims, elem_size, part_orig) );
  EXPECT_FALSE( ipn1_p1.get_tile_part_node_coor(coor, 4, part_elem_dims, elem_size, part_orig) );

  EXPECT_TRUE( ipn1_p2.get_tile_part_node_coor(coor, 2, part_elem_dims, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( coor[2], 3.0 );
  EXPECT_TRUE( ipn1_p2.get_tile_part_node_coor(coor, 3, part_elem_dims, elem_size, part_orig) );
  EXPECT_FALSE( ipn1_p2.get_tile_part_node_coor(coor, 7, part_elem_dims, elem_size, part_orig) );

  EXPECT_TRUE( ipn1_p3.get_tile_part_node_coor(coor, 2, part_elem_dims, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0/3.0 );
  EXPECT_DOUBLE_EQ( coor[1], 1.0 + 2.0/3.0 );
  EXPECT_DOUBLE_EQ( coor[2], 2.0 + 2.0/3.0 );
  EXPECT_TRUE( ipn1_p3.get_tile_part_node_coor(coor, 9, part_elem_dims, elem_size, part_orig) );
  EXPECT_FALSE( ipn1_p3.get_tile_part_node_coor(coor, 10, part_elem_dims, elem_size, part_orig) );
}

TEST( GridTileIpn1, TileElementVertexCoordinatesAOS ){
  Ipn1 ipn1;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[ipn1.get_tile_elem_n () * 2 * 3];
  // 2 vertices per element, 3 coordinates each
  EXPECT_TRUE( ipn1.get_tile_elem_vert_coor(vert_coor, 0, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 1.0 );
  EXPECT_TRUE( ipn1.get_tile_elem_vert_coor(vert_coor, 1, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 2.0 );
  EXPECT_FALSE( ipn1.get_tile_elem_vert_coor(vert_coor, 2, elem_n, elem_size, part_orig) );
}

TEST( GridTileIpn1, TileElementVertexCoordinatesSOA ){
  Ipn1 ipn1;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[ipn1.get_tile_elem_n () * 2 * 3];
  // 2 vertices per element, 3 coordinates each
  EXPECT_TRUE( ipn1.get_tile_elem_coor_vert(vert_coor, 0, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 1.0 );
  EXPECT_TRUE( ipn1.get_tile_elem_coor_vert(vert_coor, 1, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 2.0 );
  EXPECT_FALSE( ipn1.get_tile_elem_coor_vert(vert_coor, 2, elem_n, elem_size, part_orig) );
}
// Check halo nodes and elements for the Ipn1 tile
TEST( GridTileIpn1, HaloNodesAndElements){
  Tile<Ipn1> bars;
  const fmr::Local_int part_elem_dims[1] = {4};
  EXPECT_EQ( bars.get_halo_node_n(part_elem_dims), 2 );
  EXPECT_EQ( bars.get_halo_elem_n(part_elem_dims), 2 );
  EXPECT_EQ( bars.get_halo_sort_elem(0, part_elem_dims), 0 );
  EXPECT_EQ( bars.get_halo_sort_elem(1, part_elem_dims), \
    bars.get_tile_part_elem_n(part_elem_dims) - 1 );
  EXPECT_EQ( bars.get_halo_sort_elem(2, part_elem_dims), 1 );
  EXPECT_EQ( bars.get_halo_sort_elem(3, part_elem_dims), 2 );
  EXPECT_EQ( bars.get_halo_node(0, part_elem_dims), 0 );
  EXPECT_EQ( bars.get_halo_node(1, part_elem_dims), \
    bars.get_tile_part_node_n(part_elem_dims) - 1 );
}
//
} } } // end namespace femera::grid::tile
