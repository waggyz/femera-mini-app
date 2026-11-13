#include "../../femera.hpp"
#include "Bar1.hpp"
#include "../elem/Bars.hpp"// Used for quality test.

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

TEST( GridTileBar1, TrivialTest ){
  EXPECT_TRUE( true );
}
namespace femera { namespace grid { namespace tile {

TEST( GridTileBar1, IsStateless ){
  EXPECT_TRUE( (std::is_empty<Bar1>::value) );
  EXPECT_TRUE( (std::is_empty<Tile<Bar1>>::value) );
}

TEST( TileElemBar1, ElementOrderNodeCounts ){
  constexpr Tile<Bar1> linear;
  constexpr Tile<Bar1, 2> quadratic;
  constexpr Tile<Bar1, 3> cubic;

  EXPECT_EQ( linear.get_tile_vert_n (), 2 );
  EXPECT_EQ( quadratic.get_tile_vert_n (), 2 );
  EXPECT_EQ( cubic.get_tile_vert_n (), 2 );

  EXPECT_EQ( linear.get_tile_elem_node_n (), 2 );
  EXPECT_EQ( quadratic.get_tile_elem_node_n (), 3 );
  EXPECT_EQ( cubic.get_tile_elem_node_n (), 4 );
}
TEST( GridTileBar1, TileMetadata ){
  constexpr Bar1 bar1;
  EXPECT_EQ( static_cast<fmr::Local_int>(bar1.get_tile_elem_n ()), 1u );
  EXPECT_EQ( static_cast<fmr::Local_int>(bar1.get_tile_elem_node_n ()), 2u );
#if 0
  EXPECT_EQ( static_cast<fmr::Local_int>(bar1.get_tile_elem_node_n ()),
    static_cast<fmr::Local_int>(bar1.get_vert_n ()) );// P1 only
  EXPECT_EQ( static_cast<fmr::Local_int>(bar1.get_tile_vert_n ()),
    static_cast<fmr::Local_int>(bar1.get_vert_n ()) );
#endif
  EXPECT_EQ( static_cast<fmr::Local_int>(bar1.get_tile_vert_n ()), 2u );
  EXPECT_EQ( bar1.get_tile_elem_n (), 1u );
  constexpr fmr::Local_int tile0 = 0;
  constexpr fmr::Local_int tile1 = 1;
  EXPECT_EQ( tile0, 0u );
  EXPECT_EQ( tile1, 1u );
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

TEST( GridTileBar1, TilePartElemCount ){
  //get_tile_part_elem_n
  constexpr Bar1 bar1; // only [0] is used for Bar1.
  const fmr::Local_int part_elem_dims_a[3] = {2, 0, 0};
  const fmr::Local_int part_elem_dims_b[3] = {2, 3, 4};
  const fmr::Local_int part_elem_dims_c[3] = {0, 0, 0};
  const fmr::Local_int part_elem_dims_d[3] = {0, 3, 4};
  //NOTE only [0] is used for 1D elements like Bar1
  EXPECT_EQ( bar1.get_tile_part_elem_n (part_elem_dims_a), 2);
  EXPECT_EQ( bar1.get_tile_part_elem_n (part_elem_dims_b), 2);
  EXPECT_EQ( bar1.get_tile_part_elem_n (part_elem_dims_c), 0);
  EXPECT_EQ( bar1.get_tile_part_elem_n (part_elem_dims_d), 0);
}

TEST( GridTileBar1, TileVertexConnectivity ){
  constexpr Bar1 bar1_1;
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[bar1_1.get_tile_vert_n ()] = {0, 0};

  EXPECT_TRUE( bar1_1.get_tile_vert_conn (conn, 0u, part_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
  EXPECT_TRUE( bar1_1.get_tile_vert_conn (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 4 );
  EXPECT_FALSE( bar1_1.get_tile_vert_conn (conn, 4, part_size) );
}

TEST( GridTileBar1, TileVertexConnectivityPeriodic ){
  constexpr Bar1 bar1_1;// linear
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[bar1_1.get_tile_vert_n ()] = {0, 0};

  EXPECT_TRUE( bar1_1.get_tile_vert_conn_prdc (conn, 0, part_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
  EXPECT_TRUE( bar1_1.get_tile_vert_conn_prdc (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 0 );
  EXPECT_FALSE( bar1_1.get_tile_vert_conn_prdc (conn, 4, part_size) );
}

TEST( GridTileBar1, HigherOrderTileVertexConnectivity ){
  constexpr Tile<Bar1, 2> quadratic;
  constexpr Tile<Bar1, 3> cubic;
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
}

TEST( GridTileBar1, TileNodeConnectivity ){
  constexpr Tile<Bar1> linear;
  constexpr Tile<Bar1, 2> quadratic;
  constexpr Tile<Bar1, 3> cubic;
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
}

TEST( GridTileBar1, PartitionedTileVertexConnectivity ){
  constexpr Bar1 bar1;
  const fmr::Local_int part_size[1] = {3};
  fmr::Local_int conn[bar1.get_tile_vert_n ()] = {0, 0};

  EXPECT_FALSE( bar1.get_tile_vert_conn (conn, 4, part_size) );
  EXPECT_FALSE( bar1.get_tile_vert_conn (conn, 5, part_size) );
}

TEST( GridTileBar1, TileVertexCoordinates ){
  Bar1 bar1;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  //const fmr::Local_int part_node_dims[3] = {3,0,0};
  const fmr::Local_int vert_n = 3;
  fmr::Geom_float coor[3];

  EXPECT_TRUE( bar1.get_tile_vert_coor(coor, 0, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );
  EXPECT_TRUE( bar1.get_tile_vert_coor(coor, 2, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( coor[2], 2.0 );
  EXPECT_FALSE( bar1.get_tile_vert_coor(coor, 3, vert_n, elem_size, part_orig) );
}

TEST( GridTileBar1, MoreTileVertexCoordinates ){
  Bar1 bar1;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0,2.0, 3.0};
  const fmr::Local_int vert_n = 3;
  fmr::Geom_float coor[3];

  EXPECT_TRUE( bar1.get_tile_vert_coor(coor, 0, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );
  EXPECT_TRUE( bar1.get_tile_vert_coor(coor, 2, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 4.0 );
  EXPECT_DOUBLE_EQ( coor[2], 6.0 );
  EXPECT_FALSE( bar1.get_tile_vert_coor(coor, 3, vert_n, elem_size, part_orig) );
}
TEST( GridTileBar1, TileNodeCountAndCoordinates ){
  constexpr Tile<Bar1> bar1_p1;// linear default, 4 nodes in 3 elems
  constexpr Tile<Bar1, 2> bar1_p2;// 7 nodes in 3 elems
  constexpr Tile<Bar1, 3> bar1_p3;// 10 nodes in 3 elems
  const fmr::Geom_float part_orig[3] = {0.0, 1.0, 2.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int part_elem_dims[3] = {3,0,0};
  fmr::Geom_float coor[3];
  //
  EXPECT_EQ( bar1_p1.get_tile_part_node_n(part_elem_dims), 4 );
  EXPECT_EQ( bar1_p2.get_tile_part_node_n(part_elem_dims), 7 );
  EXPECT_EQ( bar1_p3.get_tile_part_node_n(part_elem_dims), 10 );
  //
  EXPECT_TRUE( bar1_p1.get_tile_part_node_coor(coor, 0, part_elem_dims, elem_size) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );// default origin
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );

  EXPECT_TRUE( bar1_p1.get_tile_part_node_coor(coor, 2, part_elem_dims, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 3.0 );
  EXPECT_DOUBLE_EQ( coor[2], 4.0 );
  EXPECT_TRUE( bar1_p1.get_tile_part_node_coor(coor, 3, part_elem_dims, elem_size, part_orig) );
  EXPECT_FALSE( bar1_p1.get_tile_part_node_coor(coor, 4, part_elem_dims, elem_size, part_orig) );

  EXPECT_TRUE( bar1_p2.get_tile_part_node_coor(coor, 2, part_elem_dims, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( coor[2], 3.0 );
  EXPECT_TRUE( bar1_p2.get_tile_part_node_coor(coor, 3, part_elem_dims, elem_size, part_orig) );
  EXPECT_FALSE( bar1_p2.get_tile_part_node_coor(coor, 7, part_elem_dims, elem_size, part_orig) );

  EXPECT_TRUE( bar1_p3.get_tile_part_node_coor(coor, 2, part_elem_dims, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0/3.0 );
  EXPECT_DOUBLE_EQ( coor[1], 1.0 + 2.0/3.0 );
  EXPECT_DOUBLE_EQ( coor[2], 2.0 + 2.0/3.0 );
  EXPECT_TRUE( bar1_p3.get_tile_part_node_coor(coor, 9, part_elem_dims, elem_size, part_orig) );
  EXPECT_FALSE( bar1_p3.get_tile_part_node_coor(coor, 10, part_elem_dims, elem_size, part_orig) );
}

TEST( GridTileBar1, TileElementVertexCoordinatesAOS ){
  Bar1 bar1;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[bar1.get_tile_elem_n () * 2 * 3];
  // 2 vertices per element, 3 coordinates each
  EXPECT_TRUE( bar1.get_tile_elem_vert_coor(vert_coor, 0, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 1.0 );
  EXPECT_TRUE( bar1.get_tile_elem_vert_coor(vert_coor, 1, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 2.0 );
  EXPECT_FALSE( bar1.get_tile_elem_vert_coor(vert_coor, 2, elem_n, elem_size, part_orig) );
}

TEST( GridTileBar1, TileElementVertexCoordinatesSOA ){
  Bar1 bar1;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[bar1.get_tile_elem_n () * 2 * 3];
  // 2 vertices per element, 3 coordinates each
  EXPECT_TRUE( bar1.get_tile_elem_coor_vert(vert_coor, 0, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 1.0 );
  EXPECT_TRUE( bar1.get_tile_elem_coor_vert(vert_coor, 1, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 2.0 );
  EXPECT_FALSE( bar1.get_tile_elem_coor_vert(vert_coor, 2, elem_n, elem_size, part_orig) );
}

TEST( GridTileBar1, TileElementEdgeDeltaQuality){
  constexpr Bar1 bar1;
  constexpr femera::grid::elem::Bars bars;

  // Use get_tile_vert_coor to get the vertex coordinates of the element.
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[bar1.get_tile_elem_n () * 2 * 3];
  // 2 vertices per element, 3 coordinates each
  EXPECT_TRUE( bar1.get_tile_elem_vert_coor(vert_coor, 0, elem_n, elem_size, part_orig) );
  const double delt_qual = bars.get_delt_qual(vert_coor, 3);
  EXPECT_GT( delt_qual, 1.0 );
  //
  const fmr::Geom_float tiny_size[3] = {1e-6, 0.0,0.0};
  const fmr::Local_int tiny_n = 1000000;
  EXPECT_TRUE( bar1.get_tile_elem_vert_coor(vert_coor, 0, tiny_n, tiny_size, part_orig) );
  const double tiny_delt_qual = bars.get_delt_qual(vert_coor, 3);
  EXPECT_GT( tiny_delt_qual, 0.0 );
  // Loop through elements and count those with "low" delta quality.
  const double qual_min = 2e-6;// just for this test, so half look bad
  fmr::Local_int low_quality_count = 0;
  for(fmr::Local_int i=0; i<tiny_n; ++i){
    EXPECT_TRUE( bar1.get_tile_elem_vert_coor(vert_coor, i, tiny_n, tiny_size, part_orig) );
    const double dq = bars.get_delt_qual(vert_coor, 3);
    if(dq < qual_min) ++low_quality_count;
  }
  // Check the return squared variant (avoids sqrt()).
  // This uses the is_squared=true flag in get_delt_qual.
  EXPECT_GT(low_quality_count, 0);
  EXPECT_EQ(low_quality_count, tiny_n / 2);
  const double qual_min2 = qual_min * qual_min;
  fmr::Local_int low_quality_count2 = 0;
  for(fmr::Local_int i=0; i<tiny_n; ++i){
    EXPECT_TRUE( bar1.get_tile_elem_vert_coor(vert_coor, i, tiny_n, tiny_size, part_orig) );
    const double dq2 = bars.get_delt_qual(vert_coor, 3, true);
    //if(dq2 < qual_min2) ++low_quality_count2;
    low_quality_count2 += (dq2 < qual_min2) ? 1 : 0;
  }
  EXPECT_GT(low_quality_count2, 0);
  EXPECT_EQ(low_quality_count2, tiny_n / 2);
}

} } } // end namespace femera::grid::tile
