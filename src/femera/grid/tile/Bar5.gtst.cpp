#include "../../femera.hpp"
#include "Bar5.hpp"
#include "Bar1.hpp"
#include "../elem/Bars.hpp"// Used for quality tests.

#include <gtest/gtest.h>

#include <type_traits>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}
namespace femera { namespace grid { namespace tile {

constexpr bool bar5_connectivity_tables_match () {
  const fmr::Local_int expected[] = {
    0,1, 1,2, 2,3, 3,0, 0,2,
    0,1, 1,2, 2,3, 3,0, 1,3
  };
  for (unsigned i = 0; i < 20; ++i) {
    if (Tile<Bar5>::get_bars_vert_conn ()[i] != expected[i]) return false;
  }
  return true;
}
static_assert (bar5_connectivity_tables_match (), "Bar5 alternate tables");

constexpr fmr::Local_int bar5_test_dims[3] = {2, 1, 0};
static_assert (Tile<Bar5>::get_tile_part_elem_n (bar5_test_dims) == 9,
  "Bar5 partition element count must be constexpr");
static_assert (Tile<Bar5, 3>::get_tile_part_node_n (bar5_test_dims) == 24,
  "Bar5 partition node count must be constexpr");
constexpr bool bar5_connectivity_constexpr () {
  fmr::Local_int conn[4] = {0, 0, 0, 0};
  return Tile<Bar5, 3>::get_tile_node_conn (conn, 5, bar5_test_dims)
    && conn[0] == 1 && conn[1] == 4 && conn[2] == 16 && conn[3] == 17;
}
static_assert (bar5_connectivity_constexpr (), "Bar5 connectivity must be constexpr");
constexpr bool bar5_coordinates_constexpr () {
  const double size[3] = {2, 3, 0};
  double coor[3] = {};
  return Tile<Bar5>::get_tile_vert_coor (coor, 4, bar5_test_dims, size)
    && coor[0] > 1 && coor[0] < 3 && coor[1] > 2 && coor[1] < 4
    && coor[2] > -1 && coor[2] < 1;
}
static_assert (bar5_coordinates_constexpr (), "Bar5 coordinates must be constexpr");

TEST( GridTileBar5, TileMetadata ){
  EXPECT_TRUE( (std::is_empty<Bar5>::value) );
  EXPECT_EQ( Tile<Bar5>::get_elem_d (), 1 );
  EXPECT_EQ( Tile<Bar5>::get_tile_d (), 2 );
  EXPECT_EQ( Tile<Bar5>::get_tile_elem_n (), 5 );
  EXPECT_EQ( Tile<Bar5>::get_tile_vert_n (), 4 );
  EXPECT_EQ( Tile<Bar5>::get_tile_edge_n (), 5 );
  EXPECT_EQ( Tile<Bar5>::tile_alts_n (), 2 );
}

TEST( GridTileBar5, BarElementNodeCounts ){
  EXPECT_EQ( Tile<Bar5>::get_tile_elem_node_n (), 2 );
  EXPECT_EQ( (Tile<Bar5, 2>::get_tile_elem_node_n ()), 3 );
  EXPECT_EQ( (Tile<Bar5, 3>::get_tile_elem_node_n ()), 4 );
  EXPECT_EQ( Tile<Bar1>::get_tile_elem_node_n (), 2 );
  EXPECT_EQ( (Tile<Bar1, 3>::get_tile_elem_node_n ()), 4 );
}

TEST( GridTileBar5, Bar1PartitionCountRegression ){
  const fmr::Local_int dims[3] = {2, 3, 4};
  const fmr::Local_int empty[3] = {0, 3, 4};
  EXPECT_EQ( Tile<Bar1>::get_tile_part_elem_n (dims), 2 );
  EXPECT_EQ( Tile<Bar1>::get_tile_part_elem_n (empty), 0 );
}

TEST( GridTileBar5, UniquePartitionElementCounts ){
  const fmr::Local_int one[3] = {1, 1, 0};
  const fmr::Local_int wide[3] = {2, 1, 0};
  const fmr::Local_int tall[3] = {1, 2, 0};
  const fmr::Local_int rectangle[3] = {2, 3, 0};
  const fmr::Local_int empty_x[3] = {0, 2, 0};
  const fmr::Local_int empty_y[3] = {2, 0, 0};
  EXPECT_EQ( Tile<Bar5>::get_tile_part_elem_n (one), 5 );
  EXPECT_EQ( Tile<Bar5>::get_tile_part_elem_n (wide), 9 );
  EXPECT_EQ( Tile<Bar5>::get_tile_part_elem_n (tall), 9 );
  EXPECT_EQ( Tile<Bar5>::get_tile_part_elem_n (rectangle), 23 );
  EXPECT_EQ( Tile<Bar5>::get_tile_part_elem_n (empty_x), 0 );
  EXPECT_EQ( Tile<Bar5>::get_tile_part_elem_n (empty_y), 0 );
}

TEST( GridTileBar5, UniquePartitionNodeCounts ){
  const fmr::Local_int one[3] = {1, 1, 0};
  const fmr::Local_int wide[3] = {2, 1, 0};
  const fmr::Local_int square[3] = {2, 2, 0};
  const fmr::Local_int empty_x[3] = {0, 2, 0};
  const fmr::Local_int empty_y[3] = {2, 0, 0};
  EXPECT_EQ( Tile<Bar5>::get_tile_part_node_n (one), 4 );
  EXPECT_EQ( (Tile<Bar5, 2>::get_tile_part_node_n (one)), 9 );
  EXPECT_EQ( (Tile<Bar5, 3>::get_tile_part_node_n (one)), 14 );
  EXPECT_EQ( Tile<Bar5>::get_tile_part_node_n (wide), 6 );
  EXPECT_EQ( (Tile<Bar5, 2>::get_tile_part_node_n (wide)), 15 );
  EXPECT_EQ( (Tile<Bar5, 3>::get_tile_part_node_n (wide)), 24 );
  EXPECT_EQ( Tile<Bar5>::get_tile_part_node_n (square), 9 );
  EXPECT_EQ( (Tile<Bar5, 2>::get_tile_part_node_n (square)), 25 );
  EXPECT_EQ( (Tile<Bar5, 3>::get_tile_part_node_n (square)), 41 );
  EXPECT_EQ( Tile<Bar5>::get_tile_part_node_n (empty_x), 0 );
  EXPECT_EQ( Tile<Bar5>::get_tile_part_node_n (empty_y), 0 );
}

TEST( GridTileBar5, CanonicalUniqueEdges ){
  const fmr::Local_int dims[3] = {2, 1, 0};
  const fmr::Local_int expected[9][2] = {
    {0,1}, {1,2}, {3,4}, {4,5}, {0,3},
    {1,4}, {2,5}, {0,4}, {1,5}
  };
  for (fmr::Local_int i = 0; i < 9; ++i) {
    fmr::Local_int vertices[2] = {99, 99}, nodes[2] = {99, 99};
    ASSERT_TRUE( Tile<Bar5>::get_tile_vert_conn (vertices, i, dims) );
    ASSERT_TRUE( Tile<Bar5>::get_tile_node_conn (nodes, i, dims) );
    for (unsigned j = 0; j < 2; ++j) {
      EXPECT_EQ( vertices[j], expected[i][j] );
      EXPECT_EQ( nodes[j], expected[i][j] );
    }
  }
}

TEST( GridTileBar5, SingleTileAlternates ){
  const fmr::Local_int dims[3] = {1,1,0};
  const fmr::Local_int expected[5][2] = {{0,1},{2,3},{0,2},{1,3},{0,3}};
  for (fmr::Local_int i = 0; i < 5; ++i) {
    fmr::Local_int conn[2] = {};
    ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn (conn, i, dims, {0,0,0})) );
    EXPECT_EQ( conn[0], expected[i][0] );
    EXPECT_EQ( conn[1], expected[i][1] );
    ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn (conn, i, dims, {1,0,0})) );
    EXPECT_EQ( conn[0], i == 4 ? 1 : expected[i][0] );
    EXPECT_EQ( conn[1], i == 4 ? 2 : expected[i][1] );
  }
}

TEST( GridTileBar5, AlternatingDiagonalsWithOffset ){
  const fmr::Local_int dims[3] = {2, 2, 0};
  const fmr::Local_int expected[4][2] = {{0,4}, {2,4}, {4,6}, {4,8}};
  const fmr::Local_int complement[4][2] = {{1,3}, {1,5}, {3,7}, {5,7}};
  for (fmr::Local_int i = 0; i < 4; ++i) {
    fmr::Local_int conn[2] = {99, 99};
    ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn (conn, 12 + i, dims, {2,0,0})) );
    EXPECT_EQ( conn[0], expected[i][0] );
    EXPECT_EQ( conn[1], expected[i][1] );
    ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn (conn, 12 + i, dims, {3,0,0})) );
    EXPECT_EQ( conn[0], complement[i][0] );
    EXPECT_EQ( conn[1], complement[i][1] );
    ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn (conn, 12 + i, dims, {2,1,0})) );
    EXPECT_EQ( conn[0], complement[i][0] );
    EXPECT_EQ( conn[1], complement[i][1] );
    ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn (conn, 12 + i, dims, {2,0,1})) );
    EXPECT_EQ( conn[0], complement[i][0] );
    EXPECT_EQ( conn[1], complement[i][1] );
  }
}

TEST( GridTileBar5, HigherOrderUniqueEdgeNodes ){
  const fmr::Local_int dims[3] = {2, 1, 0};
  const fmr::Local_int expected[4] = {1, 4, 16, 17};
  fmr::Local_int conn[4] = {99,99,99,99};
  ASSERT_TRUE( (Tile<Bar5, 3>::get_tile_node_conn (conn, 5, dims)) );
  for (unsigned j = 0; j < 4; ++j) EXPECT_EQ( conn[j], expected[j] );
  ASSERT_TRUE( (Tile<Bar5, 2>::get_tile_node_conn (conn, 8, dims, {1,0,0})) );
  EXPECT_EQ( conn[0], 2 );
  EXPECT_EQ( conn[1], 4 );
  EXPECT_EQ( conn[2], 14 );
  ASSERT_TRUE( (Tile<Bar5, 2>::get_tile_node_conn (conn, 8, dims, {1,0,0}, 3)) );
  EXPECT_EQ( conn[0], 2 );
  EXPECT_EQ( conn[1], 4 );
  EXPECT_EQ( conn[2], 22 );
  EXPECT_EQ( conn[3], 23 );
}

TEST( GridTileBar5, UniqueEdgesAndHigherOrderNodeCoverage ){
  const fmr::Local_int dims[3] = {2, 2, 0};
  bool seen_nodes[41] = {};
  bool seen_pairs[9][9] = {};
  for (fmr::Local_int edge = 0; edge < 16; ++edge) {
    fmr::Local_int nodes[4] = {};
    ASSERT_TRUE( (Tile<Bar5, 3>::get_tile_node_conn (nodes, edge, dims, {2,1,1})) );
    ASSERT_LT( nodes[0], 9 ); ASSERT_LT( nodes[1], 9 );
    const auto lo = nodes[0] < nodes[1] ? nodes[0] : nodes[1];
    const auto hi = nodes[0] < nodes[1] ? nodes[1] : nodes[0];
    EXPECT_FALSE( seen_pairs[lo][hi] );
    seen_pairs[lo][hi] = true;
    for (auto node : nodes) {
      ASSERT_LT( node, 41 );
      if (node >= 9) { EXPECT_FALSE( seen_nodes[node] ); }
      seen_nodes[node] = true;
    }
  }
  for (bool seen : seen_nodes) EXPECT_TRUE( seen );
}

TEST( GridTileBar5, CheckerboardOnOddWidthAndPartitionOffset ){
  const fmr::Local_int dims[3] = {3, 2, 0};
  const fmr::Local_int expected[6][2] = {
    {0,5}, {2,5}, {2,7}, {5,8}, {5,10}, {7,10}
  };
  for (fmr::Local_int i = 0; i < 6; ++i) {
    fmr::Local_int conn[2] = {};
    ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn (conn, 17+i, dims, {2,0,0})) );
    EXPECT_EQ( conn[0], expected[i][0] );
    EXPECT_EQ( conn[1], expected[i][1] );
  }
  const fmr::Local_int right_partition[3] = {1, 2, 0};
  fmr::Local_int left[2] = {}, right[2] = {};
  ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn (left, 19, dims, {2,0,0})) );
  ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn (right, 7, right_partition, {2,3,0})) );
  EXPECT_EQ( left[0], 2 ); EXPECT_EQ( left[1], 7 );
  EXPECT_EQ( right[0], 1 ); EXPECT_EQ( right[1], 2 );
}

TEST( GridTileBar5, ConnectivityRejectsInvalidInputs ){
  const fmr::Local_int dims[3] = {2, 1, 0};
  const fmr::Local_int empty[3] = {2, 0, 0};
  fmr::Local_int conn[4] = {99,99,99,99};
  EXPECT_FALSE( Tile<Bar5>::get_tile_vert_conn (conn, 9, dims) );
  EXPECT_FALSE( Tile<Bar5>::get_tile_node_conn (conn, 0, empty) );
  EXPECT_FALSE( (Tile<Bar5>::get_tile_vert_conn (conn, 0, dims, {4,0,0})) );
  EXPECT_FALSE( (Tile<Bar5>::get_tile_node_conn (conn, 0, dims, {0,0,0}, 0)) );
  for (auto v : conn) EXPECT_EQ( v, 99 );
}

TEST( GridTileBar5, PeriodicVertexConnectivity ){
  const fmr::Local_int dims[3] = {2, 2, 0};
  fmr::Local_int conn[2] = {99,99};
  ASSERT_TRUE( Tile<Bar5>::get_tile_vert_conn_prdc (conn, 4, dims) );
  EXPECT_EQ( conn[0], 0 ); EXPECT_EQ( conn[1], 1 );
  ASSERT_TRUE( Tile<Bar5>::get_tile_vert_conn_prdc (conn, 8, dims) );
  EXPECT_EQ( conn[0], 0 ); EXPECT_EQ( conn[1], 2 );
  ASSERT_TRUE( Tile<Bar5>::get_tile_vert_conn_prdc (conn, 15, dims) );
  EXPECT_EQ( conn[0], 3 ); EXPECT_EQ( conn[1], 0 );
  ASSERT_TRUE( (Tile<Bar5, 3>::get_tile_vert_conn_prdc (conn, 15, dims, {1,0,0})) );
  EXPECT_EQ( conn[0], 2 ); EXPECT_EQ( conn[1], 1 );
  const fmr::Local_int one[3] = {1, 1, 0};
  ASSERT_TRUE( Tile<Bar5>::get_tile_vert_conn_prdc (conn, 4, one) );
  EXPECT_EQ( conn[0], 0 ); EXPECT_EQ( conn[1], 0 );
  conn[0] = 99; conn[1] = 99;
  const fmr::Local_int empty[3] = {0, 1, 0};
  EXPECT_FALSE( Tile<Bar5>::get_tile_vert_conn_prdc (conn, 0, empty) );
  EXPECT_FALSE( (Tile<Bar5>::get_tile_vert_conn_prdc (conn, 0, dims, {4,0,0})) );
  EXPECT_EQ( conn[0], 99 ); EXPECT_EQ( conn[1], 99 );
}

TEST( GridTileBar5, OddPeriodicExtentKeepsPatternAtSeam ){
  const fmr::Local_int dims[3] = {3, 1, 0};
  fmr::Local_int left[2] = {99,99}, right[2] = {99,99};
  ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn_prdc (left, 10, dims, {2,0,0})) );
  ASSERT_TRUE( (Tile<Bar5>::get_tile_vert_conn_prdc (right, 12, dims, {2,0,0})) );
  EXPECT_EQ( left[0], 0 ); EXPECT_EQ( left[1], 1 );
  EXPECT_EQ( right[0], 2 ); EXPECT_EQ( right[1], 0 );
}

TEST( GridTileBar5, VertexCoordinates ){
  const fmr::Local_int dims[3] = {2, 1, 0};
  const double size[3] = {2, 3, 99}, origin[3] = {10, 20, 30};
  double coor[3] = {99,99,99};
  ASSERT_TRUE( Tile<Bar5>::get_tile_vert_coor (coor, 4, dims, size, origin) );
  EXPECT_DOUBLE_EQ( coor[0], 12 ); EXPECT_DOUBLE_EQ( coor[1], 23 );
  EXPECT_DOUBLE_EQ( coor[2], 30 );
  ASSERT_TRUE( Tile<Bar5>::get_tile_vert_coor (coor, 2, dims, size) );
  EXPECT_DOUBLE_EQ( coor[0], 4 ); EXPECT_DOUBLE_EQ( coor[1], 0 );
  EXPECT_DOUBLE_EQ( coor[2], 0 );
  EXPECT_FALSE( Tile<Bar5>::get_tile_vert_coor (coor, 6, dims, size) );
}

TEST( GridTileBar5, NodeCoordinatesAndLayouts ){
  const fmr::Local_int dims[3] = {2, 1, 0};
  const double size[3] = {2,3,7}, origin[3] = {10,20,30};
  double coor[3] = {}, aos[6] = {}, soa[6] = {};
  ASSERT_TRUE( (Tile<Bar5, 2>::get_tile_part_node_coor (coor, 14, dims, size, origin, {1,0,0})) );
  EXPECT_DOUBLE_EQ( coor[0], 13 ); EXPECT_DOUBLE_EQ( coor[1], 21.5 );
  EXPECT_DOUBLE_EQ( coor[2], 30 );
  ASSERT_TRUE( (Tile<Bar5>::get_tile_elem_vert_coor (aos, 8, dims, size, origin, {1,0,0})) );
  ASSERT_TRUE( (Tile<Bar5>::get_tile_elem_coor_vert (soa, 8, dims, size, origin, {1,0,0})) );
  const double expected[6] = {14,20,30, 12,23,30};
  for (unsigned i = 0; i < 6; ++i) EXPECT_DOUBLE_EQ( aos[i], expected[i] );
  for (unsigned v = 0; v < 2; ++v)
    for (unsigned d = 0; d < 3; ++d)
      EXPECT_DOUBLE_EQ( soa[2*d+v], aos[3*v+d] );
  EXPECT_FALSE( (Tile<Bar5, 2>::get_tile_part_node_coor (coor, 15, dims, size, origin)) );
}

template <fmr::Dim_int P>
void check_bar5_node_geometry () {
  const fmr::Local_int dims[3] = {2,2,0};
  const double size[3] = {2,3,7}, origin[3] = {10,20,30};
  for (fmr::Local_int variant = 0; variant < 4; ++variant) {
    const TilePattern pattern = {variant,1,0};
    for (fmr::Local_int edge = 0; edge < 16; ++edge) {
      fmr::Local_int nodes[P+1] = {};
      double aos[6] = {}, soa[6] = {};
      ASSERT_TRUE( (Tile<Bar5,P>::get_tile_node_conn (nodes, edge, dims, pattern)) );
      ASSERT_TRUE( Tile<Bar5>::get_tile_elem_vert_coor (aos, edge, dims, size, origin, pattern) );
      ASSERT_TRUE( Tile<Bar5>::get_tile_elem_coor_vert (soa, edge, dims, size, origin, pattern) );
      for (unsigned v = 0; v < 2; ++v)
        for (unsigned d = 0; d < 3; ++d)
          EXPECT_DOUBLE_EQ( aos[3*v+d], soa[2*d+v] );
      for (fmr::Local_int j = 0; j <= P; ++j) {
        double point[3] = {};
        const fmr::Local_int local = j == 0 ? 0 : (j == P ? 1 : j + 1);
        ASSERT_TRUE( (Tile<Bar5,P>::get_tile_part_node_coor
          (point, nodes[local], dims, size, origin, pattern)) );
        for (unsigned d = 0; d < 3; ++d) {
          const double expected = aos[d] + (aos[3+d] - aos[d]) * double(j) / double(P);
          EXPECT_NEAR( point[d], expected, 1e-12 );
        }
      }
    }
  }
}

TEST( GridTileBar5, AllOrdersAndPatternsCoordinateConsistency ){
  check_bar5_node_geometry<1> ();
  check_bar5_node_geometry<2> ();
  check_bar5_node_geometry<3> ();
}

TEST( GridTileBar5, InvalidCoordinatesDoNotWrite ){
  const fmr::Local_int dims[3] = {2,1,0}, empty[3] = {0,1,0};
  const double size[3] = {2,3,0};
  double coor[6] = {99,99,99,99,99,99};
  EXPECT_FALSE( Tile<Bar5>::get_tile_vert_coor (coor, 0, empty, size) );
  EXPECT_FALSE( Tile<Bar5>::get_tile_elem_vert_coor (coor, 9, dims, size) );
  EXPECT_FALSE( (Tile<Bar5>::get_tile_elem_coor_vert (coor, 0, dims, size, nullptr, {4,0,0})) );
  EXPECT_FALSE( (Tile<Bar5>::get_tile_part_node_coor (coor, 0, dims, size, nullptr, {4,0,0})) );
  for (double value : coor) EXPECT_DOUBLE_EQ( value, 99 );
}
TEST( GridTileBar5, P1P2MeshCompatibleWithFD ){
  // Compares the node coordinates of P1 and P2 Bar5 meshes with the expected
  // finite difference (FD) layout.
  const fmr::Local_int FD3x3_node_n = 3*3; // same as 2x2 element P1 Bar5 mesh
  const fmr::Local_int FD5x5_node_n = 5*5; // same as 2x2 element P2 Bar5 mesh
  const fmr::Local_int FD7x7_node_n = 7*7; // same as 2x2 element P3 Bar5 mesh
  const fmr::Local_int d2x2[3] = {2,2,0};
  //
  EXPECT_EQ( (Tile<Bar5, 1>::get_tile_part_node_n (d2x2)), FD3x3_node_n );
  EXPECT_EQ( (Tile<Bar5, 2>::get_tile_part_node_n (d2x2)), FD5x5_node_n );
  EXPECT_LT( (Tile<Bar5, 3>::get_tile_part_node_n (d2x2)), FD7x7_node_n );
  //
  double node_coor_FD3x3[FD3x3_node_n * 3]
  = {
    0.0,0.0,0.0, 1.0,0.0,0.0, 2.0,0.0,0.0,
    0.0,1.0,0.0, 1.0,1.0,0.0, 2.0,1.0,0.0,
    0.0,2.0,0.0, 1.0,2.0,0.0, 2.0,2.0,0.0
  };
  double node_coor_FD5x5[FD5x5_node_n * 3]
  = {
    0.0,0.0,0.0, 0.5,0.0,0.0, 1.0,0.0,0.0, 1.5,0.0,0.0, 2.0,0.0,0.0,
    0.0,0.5,0.0, 0.5,0.5,0.0, 1.0,0.5,0.0, 1.5,0.5,0.0, 2.0,0.5,0.0,
    0.0,1.0,0.0, 0.5,1.0,0.0, 1.0,1.0,0.0, 1.5,1.0,0.0, 2.0,1.0,0.0,
    0.0,1.5,0.0, 0.5,1.5,0.0, 1.0,1.5,0.0, 1.5,1.5,0.0, 2.0,1.5,0.0,
    0.0,2.0,0.0, 0.5,2.0,0.0, 1.0,2.0,0.0, 1.5,2.0,0.0, 2.0,2.0,0.0
  };
  // Node numbering should be tiling pattern invariant for P1 & P2.
  // Check this by comparing the computed node coordinates with the expected FD coordinates.
  double tile_node_coor[3] = {0.0, 0.0, 0.0};// init to suppress compile warning
  double tile_size[3] = {1.0, 1.0, 0.0};
  double origin[3] = {0.0, 0.0, 0.0};
  for (fmr::Local_int patvar = 0; patvar < 4; ++patvar) {
    for (fmr::Local_int offx = 0; offx < 2; ++offx) {
      for (fmr::Local_int offy = 0; offy < 2; ++offy) {
        TilePattern pattern = {patvar,offx,offy};
        for (fmr::Local_int i = 0; i < FD3x3_node_n; ++i) {
          Tile<Bar5, 1>::get_tile_part_node_coor(tile_node_coor, i, d2x2, tile_size, origin, pattern);
          for (fmr::Local_int j = 0; j < 3; ++j) {
            EXPECT_DOUBLE_EQ( tile_node_coor[j], node_coor_FD3x3[3*i + j] );
          }
        }
      }
    }
  }
  for (fmr::Local_int patvar = 0; patvar < 4; ++patvar) {
    for (fmr::Local_int offx = 0; offx < 2; ++offx) {
      for (fmr::Local_int offy = 0; offy < 2; ++offy) {
        TilePattern pattern = {patvar,offx,offy};
        for (fmr::Local_int i = 0; i < FD5x5_node_n; ++i) {
          Tile<Bar5, 2>::get_tile_part_node_coor(tile_node_coor, i, d2x2, tile_size, origin, pattern);
          for (fmr::Local_int j = 0; j < 3; ++j) {
            EXPECT_DOUBLE_EQ( tile_node_coor[j], node_coor_FD5x5[3*i + j] );
          }
        }
      }
    }
  }
}
//
} } } // end namespace femera::grid::tile
