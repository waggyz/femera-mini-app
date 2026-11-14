#include "../../femera.hpp"
#include "../Elem.test.hpp"
#include "Bars.hpp"
#include "../../../fmr/fems/elem-jacs.hpp"
#include "../../../fmr/fems/elem-poly.hpp"

#include <gtest/gtest.h>

#include <type_traits>

#undef DEBUG_PRINT

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}
namespace femera { namespace grid { namespace elem {

TEST( GridElemBars, IsStateless ){
  //EXPECT_TRUE( (std::is_empty<Bars>::value) );
  EXPECT_TRUE( (std::is_empty<Elem<Bars>>::value) );
}
TEST( GridElemBars, ElementTopologyWithoutInstance ){
  EXPECT_EQ( Elem<Bars>::get_elem_d (), 1u );
  EXPECT_EQ( Elem<Bars>::get_vert_n (), 2u );
  EXPECT_EQ( Elem<Bars>::get_edge_n (), 1u );
  EXPECT_EQ( Elem<Bars>::get_tris_n (), 0u );
  EXPECT_EQ( Elem<Bars>::get_quad_n (), 0u );
  EXPECT_EQ( Elem<Bars>::get_vols_n (), 0u );
  EXPECT_EQ( Elem<Bars>::get_face_n (), 0u );
  EXPECT_EQ( Elem<Bars>::get_node_n (), 2u );
  EXPECT_DOUBLE_EQ( Elem<Bars>::get_edge_l (), 1.0 );
  EXPECT_DOUBLE_EQ( Elem<Bars>::get_face_a (), 0.0 );
  EXPECT_DOUBLE_EQ( Elem<Bars>::get_elem_v (), 0.0 );
}
TEST( GridElemBars, ElementTopology ){
  constexpr Elem<Bars> bars;
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
  // Check can be called without creating an instance.
  EXPECT_EQ( Elem<Bars>::get_elem_p (), 1 );
  EXPECT_EQ( Elem<Bars>::get_node_n (), 2 );

  constexpr Elem<Bars> linear;
  constexpr Elem<Bars, 2> quadratic;
  constexpr Elem<Bars, 3> cubic;

  EXPECT_EQ( linear.get_elem_p (), 1 );
  EXPECT_EQ( quadratic.get_elem_p (), 2 );
  EXPECT_EQ( cubic.get_elem_p (), 3 );

  EXPECT_EQ( linear.get_node_n (), 2 );
  EXPECT_EQ( quadratic.get_node_n (), 3 );
  EXPECT_EQ( cubic.get_node_n (), 4 );
}

TEST( GridElemBars, ElementPolynomialMetadata ){
  EXPECT_EQ( Elem<Bars>::get_poly_func_size (), 2 );
  EXPECT_EQ( Elem<Bars>::get_poly_grad_size (), 2 );
  EXPECT_EQ( Elem<Bars>::get_poly_type (), fmr::math::Poly::Serendipity );
  
  const Elem<Bars> linear;
  const Elem<Bars, 2> quadratic;
  const Elem<Bars, 3> cubic;
  // Check polynomial function and gradient sizes against reference counts.
  EXPECT_EQ( linear.get_poly_func_size (), 2 );
  EXPECT_EQ( linear.get_poly_grad_size (), 2 );
  EXPECT_EQ( linear.get_poly_grad_size (4), 8 );
  EXPECT_EQ( quadratic.get_poly_func_size (), 3 );
  EXPECT_EQ( quadratic.get_poly_grad_size (), 3 );
  EXPECT_EQ( quadratic.get_poly_grad_size (4), 12 );
  EXPECT_EQ( cubic.get_poly_func_size (), 4 );
  EXPECT_EQ( cubic.get_poly_grad_size (), 4 );
  EXPECT_EQ( cubic.get_poly_grad_size (4), 16 );
  // Check polynomial type.
  EXPECT_EQ( linear.get_poly_type (), fmr::math::Poly::Serendipity );
  EXPECT_EQ( quadratic.get_poly_type (), fmr::math::Poly::Serendipity );
  EXPECT_EQ( cubic.get_poly_type (), fmr::math::Poly::Serendipity );
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
  // Bars Serendipity elements are also Full polynomial types.
  EXPECT_EQ( linear.get_node_n (),\
    fmr::math::poly_term_n (fmr::math::Poly::Full, linear.get_elem_p (),\
    linear.get_elem_d () ) );
  EXPECT_EQ( quadratic.get_node_n (),\
    fmr::math::poly_term_n (fmr::math::Poly::Full, quadratic.get_elem_p (),\
    quadratic.get_elem_d () ) );
  EXPECT_EQ( cubic.get_node_n (),\
    fmr::math::poly_term_n (fmr::math::Poly::Full, cubic.get_elem_p (),\
    cubic.get_elem_d () ) );
}
// Element connectivity and coordinates tests =================================
TEST( GridElemBars, VertexConnectivityAndCoordinates ){
  constexpr Elem<Bars> bars;

  const double* vert_coor = bars.get_elem_vert_coor ();
  const double* coor_vert = bars.get_elem_coor_vert ();
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
    for (fmr::Local_int dim = 0; dim < 3; ++dim) {
      EXPECT_DOUBLE_EQ(
        vert_coor[vertex * 3 + dim],
        coor_vert[dim * bars.get_vert_n() + vertex]);
    }
  }
}
TEST( GridElemBars, NodeCoordinates ){
  constexpr Elem<Bars> bars_p1;// linear default
  constexpr Elem<Bars, 2> bars_p2;// quadratic
  constexpr Elem<Bars, 3> bars_p3;// cubic
  const fmr::Geom_float test_vert_coor[6] = {0.0,1.0,2.0, 3.0,4.0,5.0};
  const fmr::Local_int p1_node_n = bars_p1.get_node_n();
  const fmr::Local_int p2_node_n = bars_p2.get_node_n();
  const fmr::Local_int p3_node_n = bars_p3.get_node_n();
  ASSERT_EQ(p1_node_n, 2);
  ASSERT_EQ(p2_node_n, 3);
  ASSERT_EQ(p3_node_n, 4);

  const fmr::Geom_float p1_base_node_coor[6]
    = {0.0,0.0,0.0, 1.0,0.0,0.0};
  const fmr::Geom_float p1_test_node_coor[6]
    = {0.0,1.0,2.0, 3.0,4.0,5.0};
  const fmr::Geom_float p2_base_node_coor[9]
    = {0.0,0.0,0.0, 1.0,0.0,0.0, 0.5,0.0,0.0};
  const fmr::Geom_float p2_test_node_coor[9]
    = {0.0,1.0,2.0, 3.0,4.0,5.0, 1.5,2.5,3.5};
  const fmr::Geom_float f13 = 1.0/3.0, f23 = 2.0/3.0;
  const fmr::Geom_float p3_base_node_coor[12]
    = {0.0,0.0,0.0, 1.0,0.0,0.0,
       f23*0.0+f13*1.0,0.0,0.0, f13*0.0+f23*1.0,0.0,0.0};
  const fmr::Geom_float p3_test_node_coor[12]
    = {0.0,1.0,2.0, 3.0,4.0,5.0,
       f23*0.0+f13*3.0, f23*1.0+f13*4.0, f23*2.0+f13*5.0,
       f13*0.0+f23*3.0, f13*1.0+f23*4.0, f13*2.0+f23*5.0};

  fmr::Geom_float node_coor[12] = {};
  const auto expect_node_coor = [&node_coor](
    const auto& elem, const fmr::Geom_float* expected) {
    for (fmr::Local_int i = 0; i < elem.get_node_n() * 3; ++i) {
      EXPECT_DOUBLE_EQ(node_coor[i], expected[i]);
    }
  };

  ASSERT_TRUE(bars_p1.get_elem_node_coor(node_coor));
  expect_node_coor(bars_p1, p1_base_node_coor);
  ASSERT_TRUE(bars_p1.get_elem_node_coor(node_coor, test_vert_coor));
  expect_node_coor(bars_p1, p1_test_node_coor);

  ASSERT_TRUE(bars_p2.get_elem_node_coor(node_coor));
  expect_node_coor(bars_p2, p2_base_node_coor);
  ASSERT_TRUE(bars_p2.get_elem_node_coor(node_coor, test_vert_coor));
  expect_node_coor(bars_p2, p2_test_node_coor);

  ASSERT_TRUE(bars_p3.get_elem_node_coor(node_coor));
  expect_node_coor(bars_p3, p3_base_node_coor);
  ASSERT_TRUE(bars_p3.get_elem_node_coor(node_coor, test_vert_coor));
  expect_node_coor(bars_p3, p3_test_node_coor);

  ASSERT_TRUE(Elem<Bars>::get_elem_node_coor(node_coor));
}
// Element calculations =======================================================

TEST( GridElemBars, ElementQuality ){
  const Elem<Bars> linear;
  const double* natural_coor = linear.get_elem_vert_coor ();
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
//get_delt_qual
  const double qual = linear.get_delt_qual (natural_coor, 3u);
  EXPECT_GT( qual, 0.0 );
  const double reversed_qual = linear.get_delt_qual (reversed_coor, 3u);
  EXPECT_DOUBLE_EQ( reversed_qual, qual );
  const double zero_qual = linear.get_delt_qual (zero_coor, 3u);
  EXPECT_DOUBLE_EQ( zero_qual, 0.0 );
  const double bad_qual = linear.get_delt_qual (bad_coor, 3u);
  EXPECT_GT( bad_qual, 0.0 );
  EXPECT_LT( bad_qual, qual );

  EXPECT_GT( Elem<Bars>::get_delt_qual (natural_coor, 3u), 0.0 );
  //TODO more checks here
}
#if 1
TEST( GridElemBars, ShapeFunctionsAndGradients ){
  const double xi[3] = {0.5, 0.0, 0.0};
  double N[2];
  double dN[2];
  fmr::fems::elem::calc_poly_func_C1D2N(N, xi);
  fmr::fems::elem::calc_poly_grad_node_C1D2N(dN, xi);
  EXPECT_DOUBLE_EQ(N[0], 0.5);
  EXPECT_DOUBLE_EQ(N[1], 0.5);
  EXPECT_DOUBLE_EQ(dN[0], -1.0);
  EXPECT_DOUBLE_EQ(dN[1], 1.0);
}
TEST( GridElemBars, JacobianCalculation ){
  const double xi[3] = {0.5, 0.0, 0.0};
  double gradient[2];
  double gradient_ref_major[2];
  fmr::fems::elem::calc_poly_grad_node_C1D2N(gradient, xi);
  fmr::fems::elem::calc_poly_node_grad_C1D2N(gradient_ref_major, xi);
  EXPECT_DOUBLE_EQ(gradient_ref_major[0], gradient[0]);
  EXPECT_DOUBLE_EQ(gradient_ref_major[1], gradient[1]);

  const double origin[3] = {2.0, 3.0, 5.0};
  const double edge[3] = {1.0, 0.0, 0.0};
  for(fmr::Dim_int sims_d=1; sims_d<=3; ++sims_d){
    SCOPED_TRACE(sims_d);
    double node_coor[6] = {};
    double coor_node[6] = {};
    for(fmr::Local_int k=0; k<2; ++k){
      for(fmr::Dim_int i=0; i<sims_d; ++i){
        node_coor[k*sims_d+i] = origin[i] + k*edge[i];
        coor_node[i*2+k] = node_coor[k*sims_d+i];
      }
    }
    fmr::Phys_float jac_aos[4] = {};
    fmr::Phys_float jac_soa[4] = {};
    fmr::Phys_float jac_ref_major[4] = {};
    jac_aos[sims_d] = jac_soa[sims_d] = jac_ref_major[sims_d] = -99.0;
    fmr::fems::elem::elem_jacs_node_coor_ref(
      jac_aos, node_coor, gradient, 1u, 2u, sims_d, 1u);
    fmr::fems::elem::elem_jacs_coor_node_ref(
      jac_soa, coor_node, gradient, 1u, 2u, sims_d, 1u);
    fmr::fems::elem::elem_jacs_node_coor_a(
      jac_ref_major, node_coor, gradient_ref_major, 1u, 2u, sims_d, 1u);
    for(fmr::Dim_int i=0; i<sims_d; ++i){
      EXPECT_DOUBLE_EQ(jac_aos[i], edge[i]);
      EXPECT_DOUBLE_EQ(jac_soa[i], edge[i]);
      EXPECT_DOUBLE_EQ(jac_ref_major[i], edge[i]);
    }
    EXPECT_DOUBLE_EQ(jac_aos[sims_d], -99.0);
    EXPECT_DOUBLE_EQ(jac_soa[sims_d], -99.0);
    EXPECT_DOUBLE_EQ(jac_ref_major[sims_d], -99.0);
  }
}
#endif
#if 1
TEST( GridElemBars, JacobianCalculationMultiplePointsAndDimensions ){
  constexpr fmr::Local_int intp_n = 2;
  const double origin[3] = {2.0, 3.0, 5.0};
  const double edge[3] = {5.0, 8.0, 11.0};
  const double gradients[4] = {-1.0, 1.0, -1.0, 1.0};
  const double gradients_ref_major[4] = {-1.0, 1.0, -1.0, 1.0};
  for(fmr::Dim_int sims_d=1; sims_d<=3; ++sims_d){
    SCOPED_TRACE(sims_d);
    double node_coor[6] = {};
    double coor_node[6] = {};
    fmr::Phys_float jacs_aos[7] = {};
    fmr::Phys_float jacs_soa[7] = {};
    fmr::Phys_float jacs_ref_major[7] = {};
    for(fmr::Local_int k=0; k<2; ++k){
      for(fmr::Dim_int i=0; i<sims_d; ++i){
        node_coor[k*sims_d + i] = origin[i] + k*edge[i];
        coor_node[i*2 + k] = node_coor[k*sims_d + i];
      }
    }
    jacs_aos[intp_n*sims_d] = -99.0;
    jacs_soa[intp_n*sims_d] = -99.0;
    jacs_ref_major[intp_n*sims_d] = -99.0;
    fmr::fems::elem::elem_jacs_node_coor_ref(
      jacs_aos, node_coor, gradients, intp_n, 2u, sims_d, 1u);
    fmr::fems::elem::elem_jacs_coor_node_ref(
      jacs_soa, coor_node, gradients, intp_n, 2u, sims_d, 1u);
    fmr::fems::elem::elem_jacs_node_coor_a(
      jacs_ref_major, node_coor, gradients_ref_major, intp_n, 2u, sims_d, 1u);
    for(fmr::Local_int ip=0; ip<intp_n; ++ip){
      for(fmr::Dim_int i=0; i<sims_d; ++i){
        const auto index = ip*sims_d+i;
        EXPECT_DOUBLE_EQ(jacs_aos[index], edge[i]);
        EXPECT_DOUBLE_EQ(jacs_soa[index], edge[i]);
        EXPECT_DOUBLE_EQ(jacs_ref_major[index], edge[i]);
      }
    }
    EXPECT_DOUBLE_EQ(jacs_aos[intp_n*sims_d], -99.0);
    EXPECT_DOUBLE_EQ(jacs_soa[intp_n*sims_d], -99.0);
    EXPECT_DOUBLE_EQ(jacs_ref_major[intp_n*sims_d], -99.0);
  }
}
#endif
} } } // end namespace femera::grid::elem
#undef DEBUG_PRINT