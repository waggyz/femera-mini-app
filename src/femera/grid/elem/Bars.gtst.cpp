#include "../../femera.hpp"
#include "../Elem.test.hpp"
#include "Bars.hpp"
#include "../../../fmr/fems/elem.hpp"

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
  constexpr Bars bars;
#if 0
  EXPECT_EQ( static_cast<fmr::Local_int>(Elem<Bars>::get_sims_d ()), 3u );
  EXPECT_EQ( static_cast<fmr::Local_int>(Elem<Bars>::get_elem_d ()), 1u );
#endif
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
  constexpr Elem<Bars> linear;
  constexpr Elem<Bars, 2> quadratic;
  constexpr Elem<Bars, 3> cubic;

  EXPECT_EQ( linear.get_elem_p (), 1 );
  EXPECT_EQ( quadratic.get_elem_p (), 2 );
  EXPECT_EQ( cubic.get_elem_p (), 3 );

  EXPECT_EQ( linear.get_node_n (), 2 );
  EXPECT_EQ( quadratic.get_node_n (), 3 );
  EXPECT_EQ( cubic.get_node_n (), 4 );

  EXPECT_EQ( linear.get_tile_vert_n (), 2 );
  EXPECT_EQ( quadratic.get_tile_vert_n (), 2 );
  EXPECT_EQ( cubic.get_tile_vert_n (), 2 );

  EXPECT_EQ( linear.get_tile_elem_node_n (), 2 );
  EXPECT_EQ( quadratic.get_tile_elem_node_n (), 3 );
  EXPECT_EQ( cubic.get_tile_elem_node_n (), 4 );
}

TEST( GridElemBars, ElementPolynomials ){
  const Elem<Bars> linear;
  const Elem<Bars, 2> quadratic;
  const Elem<Bars, 3> cubic;
  // Check polynomial function and gradient sizes against reference counts.
  EXPECT_EQ( linear.get_poly_func_size (), 2 );
  EXPECT_EQ( linear.get_poly_grad_size (), 6 );//TODO may be incorrect for bar in 3D
  EXPECT_EQ( quadratic.get_poly_func_size (), 3 );
  EXPECT_EQ( quadratic.get_poly_grad_size (), 9 );//TODO may be incorrect for bar in 3D
  EXPECT_EQ( cubic.get_poly_func_size (), 4 );
  EXPECT_EQ( cubic.get_poly_grad_size (), 12 );//TODO may be incorrect for bar in 3D
  // Check polynomial type.
  EXPECT_EQ( linear.get_poly_type (), fmr::math::Poly::Full );
  EXPECT_EQ( quadratic.get_poly_type (), fmr::math::Poly::Full );
  EXPECT_EQ( cubic.get_poly_type (), fmr::math::Poly::Full );
  // Check node_n matches polynomial function size.
  EXPECT_EQ( linear.get_node_n (), linear.get_poly_func_size () );
  EXPECT_EQ( quadratic.get_node_n (), quadratic.get_poly_func_size () );
  EXPECT_EQ( cubic.get_node_n (), cubic.get_poly_func_size () );
  // Check node_n matches poly term count.
  EXPECT_EQ( linear.get_node_n (),\
    fmr::math::poly_term_n (linear.get_poly_type (), linear.get_elem_p (),\
    linear.get_elem_d () ) );
  EXPECT_EQ( quadratic.get_node_n (),\
    fmr::math::poly_term_n (quadratic.get_poly_type (), quadratic.get_elem_p (),\
    quadratic.get_elem_d () ) );
  EXPECT_EQ( cubic.get_node_n (),\
    fmr::math::poly_term_n (cubic.get_poly_type (), cubic.get_elem_p (),\
    cubic.get_elem_d () ) );
}

TEST( GridElemBars, VertexConnectivityAndCoordinates ){
  constexpr Bars bars;
#if 0
  ASSERT_NE( bars.get_vert_conn (), nullptr );
  constexpr fmr::Local_int vert0 = Bars::vert_conn[0];
  constexpr fmr::Local_int vert1 = Bars::vert_conn[1];
  EXPECT_EQ( vert0, 0u );
  EXPECT_EQ( vert1, 1u );
  EXPECT_EQ( bars.get_vert_conn ()[0], 0 );
  EXPECT_EQ( bars.get_vert_conn ()[1], 1 );
#endif

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

  for (fmr::Local_int vertex = 0; vertex < bars.get_vert_n(); ++vertex) {
    for (fmr::Local_int dim = 0; dim < bars.get_sims_d(); ++dim) {
      EXPECT_DOUBLE_EQ(
        vert_coor[vertex * bars.get_sims_d() + dim],
        coor_vert[dim * bars.get_vert_n() + vertex]);
    }
  }
}

TEST( GridElemBars, ElementQuality ){
  const Elem<Bars> linear;
  const double* natural_coor = linear.get_vert_coor ();
  //
  // reversed order of nodes for testing purposes
  const double* reversed_coor = []{
    static double arr[6] = {1.0,0.0,0.0, 0.0,0.0,0.0};
    return arr;
  }();
  const double* zero_coor = []{
    static double arr[6] = {0.0,0.0,0.0, 0.0,0.0,0.0};
    return arr;
  }();
  const double* bad_coor = []{
    static double arr[6] = {1.0,0.0,0.0, 1.0+1e-7,0.0,0.0};
    return arr;
  }();
  ASSERT_NE( natural_coor, nullptr );
  ASSERT_NE( zero_coor, nullptr );
  ASSERT_NE( reversed_coor, nullptr );
  ASSERT_NE( bad_coor, nullptr );
//calc_delt_qual
  const double qual = linear.calc_delt_qual (natural_coor, 3u);
  EXPECT_GT( qual, 0.0 );
  const double reversed_qual = linear.calc_delt_qual (reversed_coor, 3u);
  EXPECT_DOUBLE_EQ( reversed_qual, qual );
  const double zero_qual = linear.calc_delt_qual (zero_coor, 3u);
  EXPECT_DOUBLE_EQ( zero_qual, 0.0 );
  const double bad_qual = linear.calc_delt_qual (bad_coor, 3u);
  EXPECT_GT( bad_qual, 0.0 );
  EXPECT_LT( bad_qual, qual );
  //TODO more checks here
  //
  //NOTE See TileElementEdgeDeltaQuality tests below for tiling checks.
}
#if 1
TEST( GridElemBars, ShapeFunctionsAndGradients ){
  constexpr Bars bars;// Default linear 2-node element
  const double xi = 0.5;// int point for Gauss Legendre first order quadrature
  double N_storage[2];
  double dN_storage[2 * 3];// Default 3D space
  double* N = N_storage;
  double* dN = dN_storage;
  bars.calc_poly_func_p1 (N, &xi);
  bars.calc_poly_grad_p1 (dN, &xi);
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
#endif
TEST( GridElemBars, JacobianCalculation ){
  constexpr Bars bars;
  const double xi = 0.5;
  double dN_storage[2 * 3];
  double* dN = dN_storage;
  double dNT_storage[2 * 3];
  double* dNT = dNT_storage;
  bars.calc_poly_grad_p1 (dN, &xi);
  bars.calc_poly_grat_p1 (dNT, &xi);
  fmr::Phys_float jacs_caos_storage[3 * 3];
  fmr::Phys_float* jacs_caos = jacs_caos_storage;
  fmr::Phys_float jacs_caos_a_storage[3 * 3];
  fmr::Phys_float* jacs_caos_a = jacs_caos_a_storage;
  fmr::Phys_float jacs_csoa_storage[3 * 3];
  fmr::Phys_float* jacs_csoa = jacs_csoa_storage;
  const double* node_coor = bars.get_vert_coor ();// naos
  const double* coor_node = bars.get_coor_vert ();// nsoa
  //
  fmr::fems::elem::calc_jacs_caos_ref (jacs_caos, node_coor, dNT, 1u, 2u, 3u);// coor aos
  fmr::fems::elem::calc_jacs_caos_a   (jacs_caos_a, node_coor, dN, 1u, 2u, 3u);// coor aos
  fmr::fems::elem::calc_jacs_csoa_ref (jacs_csoa, coor_node, dNT, 1u, 2u, 3u);// coor soa
  //
  // A bar along the natural x-axis has only a nonzero x derivative.
  //TODO FIXME incorrect Jacobian for 1D element embedded in higher dimensions.
  EXPECT_DOUBLE_EQ( jacs_caos[0], 1.0 );
  EXPECT_DOUBLE_EQ( jacs_caos[1], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_caos[2], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_caos[3], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_caos[4], 0.0 );// 1.0
  EXPECT_DOUBLE_EQ( jacs_caos[5], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_caos[6], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_caos[7], 0.0 );
  EXPECT_DOUBLE_EQ( jacs_caos[8], 0.0 );// 1.0
  //
  EXPECT_DOUBLE_EQ( jacs_csoa[0], jacs_caos[0] );
  EXPECT_DOUBLE_EQ( jacs_csoa[1], jacs_caos[1] );
  EXPECT_DOUBLE_EQ( jacs_csoa[2], jacs_caos[2] );
  EXPECT_DOUBLE_EQ( jacs_csoa[3], jacs_caos[3] );
  EXPECT_DOUBLE_EQ( jacs_csoa[4], jacs_caos[4] );
  EXPECT_DOUBLE_EQ( jacs_csoa[5], jacs_caos[5] );
  EXPECT_DOUBLE_EQ( jacs_csoa[6], jacs_caos[6] );
  EXPECT_DOUBLE_EQ( jacs_csoa[7], jacs_caos[7] );
  EXPECT_DOUBLE_EQ( jacs_csoa[8], jacs_caos[8] );
  //
  EXPECT_DOUBLE_EQ( jacs_caos_a[0], jacs_caos[0] );
  EXPECT_DOUBLE_EQ( jacs_caos_a[1], jacs_caos[1] );
  EXPECT_DOUBLE_EQ( jacs_caos_a[2], jacs_caos[2] );
  EXPECT_DOUBLE_EQ( jacs_caos_a[3], jacs_caos[3] );
  EXPECT_DOUBLE_EQ( jacs_caos_a[4], jacs_caos[4] );
  EXPECT_DOUBLE_EQ( jacs_caos_a[5], jacs_caos[5] );
  EXPECT_DOUBLE_EQ( jacs_caos_a[6], jacs_caos[6] );
  EXPECT_DOUBLE_EQ( jacs_caos_a[7], jacs_caos[7] );
  EXPECT_DOUBLE_EQ( jacs_caos_a[8], jacs_caos[8] );
}

TEST( GridElemBars, JacobianCalculationMultiplePointsAndDimensions ){
  constexpr Bars bars;
  constexpr fmr::Local_int vert_n = bars.get_vert_n ();
  constexpr fmr::Local_int intp_n = 2;
  const double origin[3] = {2.0, 3.0, 5.0};
  const double edge[3] = {5.0, 8.0, 11.0};
  for(fmr::Local_int dims=1; dims<=3; ++dims){
    SCOPED_TRACE(dims);
    double vert_coor[2 * 3];
    double coor_vert[2 * 3];
    double poly_grad[2 * 2 * 3];
    double poly_grat[2 * 2 * 3];
    fmr::Phys_float jacs_caos[2 * 3 * 3];
    fmr::Phys_float jacs_caos_a[2 * 3 * 3];
    fmr::Phys_float jacs_csoa[2 * 3 * 3];
    for(fmr::Local_int k=0; k<vert_n; ++k){
      for(fmr::Local_int i=0; i<dims; ++i){
        vert_coor[k*dims + i] = origin[i] + k*edge[i];
        coor_vert[i*vert_n + k] = vert_coor[k*dims + i];
      }
    }
    for(fmr::Local_int ip=0; ip<intp_n; ++ip){
      for(fmr::Local_int j=0; j<dims; ++j){
        const double weight = static_cast<double>(ip*dims + j + 1);
        for(fmr::Local_int k=0; k<vert_n; ++k){
          const double gradient = k==0 ? -weight : weight;
          poly_grad[ip*vert_n*dims + k*dims + j] = gradient;
          poly_grat[ip*vert_n*dims + j*vert_n + k]
            = gradient;
        }
      }
    }
    fmr::fems::elem::calc_jacs_caos_ref (jacs_caos, vert_coor, poly_grad, intp_n, vert_n, dims);
    fmr::fems::elem::calc_jacs_caos_a (jacs_caos_a, vert_coor, poly_grat, intp_n, vert_n, dims);
    fmr::fems::elem::calc_jacs_csoa_ref (jacs_csoa, coor_vert, poly_grad, intp_n, vert_n, dims);
    for(fmr::Local_int ip=0; ip<intp_n; ++ip){
      for(fmr::Local_int i=0; i<dims; ++i){
        for(fmr::Local_int j=0; j<dims; ++j){
          const auto index = ip*dims*dims + i*dims + j;
          const double expected = edge[i] * (ip*dims + j + 1);
          EXPECT_DOUBLE_EQ( jacs_caos[index], expected );
          EXPECT_DOUBLE_EQ( jacs_caos_a[index], expected );
          EXPECT_DOUBLE_EQ( jacs_csoa[index], expected );
        }
      }
    }
  }
}

TEST( GridElemBars, TileMetadata ){
  constexpr Bars bars;
  EXPECT_EQ( static_cast<fmr::Local_int>(bars.get_tile_elem_n ()), 1u );
  EXPECT_EQ( static_cast<fmr::Local_int>(bars.get_tile_elem_node_n ()),
    static_cast<fmr::Local_int>(bars.get_vert_n ()) );// P1 only
  EXPECT_EQ( static_cast<fmr::Local_int>(bars.get_tile_elem_node_n ()), 2u );
  EXPECT_EQ( static_cast<fmr::Local_int>(bars.get_tile_vert_n ()),
    static_cast<fmr::Local_int>(bars.get_vert_n ()) );
  EXPECT_EQ( static_cast<fmr::Local_int>(bars.get_tile_vert_n ()), 2u );
  EXPECT_EQ( bars.get_tile_elem_n (), 1u );
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
  const Elem<TileDispatchTestShape, 3> shape;
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[1] = {0};
  return shape.get_tile_vert_conn (conn, 2, part_size) && conn[0] == 5;
}
static_assert (tile_dispatch_constexpr_test (), "Tile dispatch must be constexpr");

TEST( GridElemBars, TileVertexConnectivity ){
  constexpr Bars bars1;
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[bars1.get_tile_vert_n ()] = {0, 0};

  EXPECT_TRUE( bars1.get_tile_vert_conn (conn, 0u, part_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
  EXPECT_TRUE( bars1.get_tile_vert_conn (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 4 );
  EXPECT_FALSE( bars1.get_tile_vert_conn (conn, 4, part_size) );
}

TEST( GridElemBars, TileVertexConnectivityPeriodic ){
  constexpr Bars bars1;// linear
  const fmr::Local_int part_size[1] = {4};
  fmr::Local_int conn[bars1.get_tile_vert_n ()] = {0, 0};

  EXPECT_TRUE( bars1.get_tile_vert_conn_prdc (conn, 0, part_size) );
  EXPECT_EQ( conn[0], 0 );
  EXPECT_EQ( conn[1], 1 );
  EXPECT_TRUE( bars1.get_tile_vert_conn_prdc (conn, 3, part_size) );
  EXPECT_EQ( conn[0], 3 );
  EXPECT_EQ( conn[1], 0 );
  EXPECT_FALSE( bars1.get_tile_vert_conn_prdc (conn, 4, part_size) );
}

TEST( GridElemBars, HigherOrderTileVertexConnectivity ){
  constexpr Elem<Bars, 2> quadratic;
  constexpr Elem<Bars, 3> cubic;
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

TEST( GridElemBars, TileNodeConnectivity ){
  constexpr Elem<Bars> linear;
  constexpr Elem<Bars, 2> quadratic;
  constexpr Elem<Bars, 3> cubic;
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

TEST( GridElemBars, PartitionedTileVertexConnectivity ){
  constexpr Bars bars;
  const fmr::Local_int part_size[1] = {3};
  fmr::Local_int conn[bars.get_tile_vert_n ()] = {0, 0};

  EXPECT_FALSE( bars.get_tile_vert_conn (conn, 4, part_size) );
  EXPECT_FALSE( bars.get_tile_vert_conn (conn, 5, part_size) );
}

TEST( GridElemBars, TileVertexCoordinates ){
  Bars bars;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int vert_n = 3;
  fmr::Geom_float coor[3];

  EXPECT_TRUE( bars.get_tile_vert_coor(coor, 0, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );
  EXPECT_TRUE( bars.get_tile_vert_coor(coor, 2, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( coor[2], 2.0 );
  EXPECT_FALSE( bars.get_tile_vert_coor(coor, 3, vert_n, elem_size, part_orig) );
#if 0
  EXPECT_TRUE( bars.get_tile_node_coor(coor, 2, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( coor[2], 2.0 );
  EXPECT_FALSE( bars.get_tile_node_coor(coor, 3, vert_n, elem_size, part_orig) );
#endif
}
TEST( GridElemBars, MoreTileVertexCoordinates ){
  Bars bars;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0,2.0, 3.0};
  const fmr::Local_int vert_n = 3;
  fmr::Geom_float coor[3];

  EXPECT_TRUE( bars.get_tile_vert_coor(coor, 0, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( coor[2], 0.0 );
  EXPECT_TRUE( bars.get_tile_vert_coor(coor, 2, vert_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( coor[0], 2.0 );
  EXPECT_DOUBLE_EQ( coor[1], 4.0 );
  EXPECT_DOUBLE_EQ( coor[2], 6.0 );
  EXPECT_FALSE( bars.get_tile_vert_coor(coor, 3, vert_n, elem_size, part_orig) );
}

TEST( GridElemBars, TileElementVertexCoordinatesAOS ){
  Bars bars;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[bars.get_tile_elem_n () * 2 * 3];
  // 2 vertices per element, 3 coordinates each
  EXPECT_TRUE( bars.get_tile_elem_vert_coor(vert_coor, 0, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 1.0 );
  EXPECT_TRUE( bars.get_tile_elem_vert_coor(vert_coor, 1, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 2.0 );
  EXPECT_FALSE( bars.get_tile_elem_vert_coor(vert_coor, 2, elem_n, elem_size, part_orig) );
}

TEST( GridElemBars, TileElementVertexCoordinatesSOA ){
  Bars bars;
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[bars.get_tile_elem_n () * 2 * 3];
  // 2 vertices per element, 3 coordinates each
  EXPECT_TRUE( bars.get_tile_elem_coor_vert(vert_coor, 0, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 0.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 1.0 );
  EXPECT_TRUE( bars.get_tile_elem_coor_vert(vert_coor, 1, elem_n, elem_size, part_orig) );
  EXPECT_DOUBLE_EQ( vert_coor[0], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[1], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[2], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[3], 2.0 );
  EXPECT_DOUBLE_EQ( vert_coor[4], 1.0 );
  EXPECT_DOUBLE_EQ( vert_coor[5], 2.0 );
  EXPECT_FALSE( bars.get_tile_elem_coor_vert(vert_coor, 2, elem_n, elem_size, part_orig) );
}

TEST( GridElemBars, TileElementEdgeDeltaQuality){
  constexpr Bars bars;
  // use get_tile_vert_coor to get the vertex coordinates of the element
  const fmr::Geom_float part_orig[3] = {0.0, 0.0, 0.0};
  const fmr::Geom_float elem_size[3] = {1.0, 1.0, 1.0};
  const fmr::Local_int elem_n = 2;
  fmr::Geom_float vert_coor[bars.get_tile_elem_n () * 2 * 3];
  // 2 vertices per element, 3 coordinates each
  EXPECT_TRUE( bars.get_tile_elem_vert_coor(vert_coor, 0, elem_n, elem_size, part_orig) );
  const double delt_qual = bars.calc_delt_qual(vert_coor, 3);
  EXPECT_GT( delt_qual, 1.0 );
  //
  const fmr::Geom_float tiny_size[3] = {1e-6, 0.0,0.0};
  const fmr::Local_int tiny_n = 1000000;
  EXPECT_TRUE( bars.get_tile_elem_vert_coor(vert_coor, 0, tiny_n, tiny_size, part_orig) );
  const double tiny_delt_qual = bars.calc_delt_qual(vert_coor, 3);
  EXPECT_GT( tiny_delt_qual, 0.0 );
  // Loop through elements and count those with "low" delta quality.
  fmr::Local_int low_quality_count = 0;
  for(fmr::Local_int i=0; i<tiny_n; ++i){
    EXPECT_TRUE( bars.get_tile_elem_vert_coor(vert_coor, i, tiny_n, tiny_size, part_orig) );
    const double dq = bars.calc_delt_qual(vert_coor, 3);
    if(dq < 2e-6) ++low_quality_count;// just for this test, so half look bad
  }
  EXPECT_GT(low_quality_count, 0);
  EXPECT_EQ(low_quality_count, tiny_n / 2);
}

} } } // end namespace femera::grid::elem
