#include "../../femera.hpp"
#include "../Elem.test.hpp"
#include "Bars.hpp"
#include "../../../fmr/fems/elem-jacs.hpp"
#include "../../../fmr/fems/elem-poly.hpp"
#include "../../../fmr/smat.hpp"

#include <gtest/gtest.h>

#include <type_traits>

#undef DEBUG_PRINT

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}
namespace femera { namespace grid { namespace elem {

TEST( GridElemBars, IsStatelessAndPrivate ){
  EXPECT_TRUE( (std::is_empty<Elem<Bars>>::value) );
  EXPECT_TRUE( (std::is_empty<Bars>::value) );// private constructor
  EXPECT_FALSE( (std::is_default_constructible<Bars>::value) );
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
  EXPECT_EQ( Elem<Bars>::get_poly_type (), fmr::math::Poly::Full );
  //
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

TEST( GridElemBars, ElementPolyGradEval ){
  const Elem<Bars> linear;
  const Elem<Bars, 2> quadratic;
  const Elem<Bars, 3> cubic;
  // Check polynomial functions at coor {0.5}
  const fmr::Geom_float intp_coor[1] = {0.5};
  const auto poly_1_size = linear.get_poly_func_size ();
  const auto poly_2_size = quadratic.get_poly_func_size ();
  const auto poly_3_size = cubic.get_poly_func_size ();
  const auto grad_1_size = linear.get_poly_grad_size ();
  const auto grad_2_size = quadratic.get_poly_grad_size ();
  const auto grad_3_size = cubic.get_poly_grad_size ();
  double poly_1[poly_1_size];
  double poly_2[poly_2_size];
  double poly_3[poly_3_size];
  double grad_1[grad_1_size];
  double grad_2[grad_2_size];
  double grad_3[grad_3_size];
  linear.get_poly_func_vals (poly_1, &intp_coor[0] );
  linear.get_poly_node_grad (grad_1, &intp_coor[0] );
  quadratic.get_poly_func_vals (poly_2, &intp_coor[0] );
  quadratic.get_poly_node_grad (grad_2, &intp_coor[0] );
  cubic.get_poly_func_vals (poly_3, &intp_coor[0] );
  cubic.get_poly_node_grad (grad_3, &intp_coor[0] );
  EXPECT_DOUBLE_EQ( poly_1[0], 0.5 );
  EXPECT_DOUBLE_EQ( poly_1[1], 0.5 );
  //
  EXPECT_DOUBLE_EQ( poly_2[0], 0.0 );
  EXPECT_DOUBLE_EQ( poly_2[1], 0.0 );
  EXPECT_DOUBLE_EQ( poly_2[2], 1.0 );
  //
  EXPECT_DOUBLE_EQ( poly_3[0],-0.0625 );
  EXPECT_DOUBLE_EQ( poly_3[1],-0.0625 );
  EXPECT_DOUBLE_EQ( poly_3[2], 0.5625 );
  EXPECT_DOUBLE_EQ( poly_3[3], 0.5625 );
  //
  EXPECT_DOUBLE_EQ( grad_1[0],-1.0 );
  EXPECT_DOUBLE_EQ( grad_1[1], 1.0 );
  //
  EXPECT_DOUBLE_EQ( grad_2[0],-1.0 );
  EXPECT_DOUBLE_EQ( grad_2[1], 1.0 );
  EXPECT_DOUBLE_EQ( grad_2[2], 0.0 );
  //
  EXPECT_DOUBLE_EQ( grad_3[0], 0.125 );
  EXPECT_DOUBLE_EQ( grad_3[1],-0.125 );
  EXPECT_DOUBLE_EQ( grad_3[2],-3.375 );
  EXPECT_DOUBLE_EQ( grad_3[3], 3.375 );
}
TEST( GridElemBars, ElementPolyEvalFloat ){
  const Elem<Bars, 1, float> linear;
  const Elem<Bars, 2, float> quadratic;
  const Elem<Bars, 3, float> cubic;
// Test at natural element ordinates.
  const float test_coor[5] = {0.0f, 1.0f, 0.5f, 1.0f/3.0f, 2.0f/3.0f};
  float test_poly[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  const float eps = std::numeric_limits<float>::epsilon();
  linear.get_poly_func_vals (test_poly, &test_coor[0] );
  EXPECT_FLOAT_EQ( test_poly[0], 1.0f );
  EXPECT_LE( abs(test_poly[1]), eps );
  linear.get_poly_func_vals (test_poly, &test_coor[1] );
  EXPECT_LE( abs(test_poly[0]), eps );
  EXPECT_FLOAT_EQ( test_poly[1], 1.0f );
  linear.get_poly_func_vals (test_poly, &test_coor[2] );
  EXPECT_FLOAT_EQ( test_poly[0], 0.5f );
  EXPECT_FLOAT_EQ( test_poly[1], 0.5f );
  // Linear is constant gradient.
  linear.get_poly_node_grad (test_poly, &test_coor[0] );
  EXPECT_FLOAT_EQ( test_poly[0],-1.0f );
  EXPECT_FLOAT_EQ( test_poly[1], 1.0f );
  linear.get_poly_node_grad (test_poly, &test_coor[1] );
  EXPECT_FLOAT_EQ( test_poly[0],-1.0f );
  EXPECT_FLOAT_EQ( test_poly[1], 1.0f );
  linear.get_poly_node_grad (test_poly, &test_coor[2] );
  EXPECT_FLOAT_EQ( test_poly[0],-1.0f );
  EXPECT_FLOAT_EQ( test_poly[1], 1.0f );
  //
  quadratic.get_poly_func_vals (test_poly, &test_coor[0] );
  EXPECT_FLOAT_EQ( test_poly[0], 1.0f );
  EXPECT_LE( abs(test_poly[1]), eps );
  EXPECT_LE( abs(test_poly[2]), eps );
  quadratic.get_poly_func_vals (test_poly, &test_coor[1] );
  EXPECT_LE( abs(test_poly[0]), eps );
  EXPECT_FLOAT_EQ( test_poly[1], 1.0f );
  EXPECT_FLOAT_EQ( test_poly[2], 0.0f );
  quadratic.get_poly_func_vals (test_poly, &test_coor[2] );
  EXPECT_FLOAT_EQ( test_poly[0], 0.0f );
  EXPECT_FLOAT_EQ( test_poly[1], 0.0f );
  EXPECT_FLOAT_EQ( test_poly[2], 1.0f );
  //
  cubic.get_poly_func_vals (test_poly, &test_coor[0] );
  EXPECT_FLOAT_EQ( test_poly[0], 1.0f );
  EXPECT_LE( abs(test_poly[1]), eps );
  EXPECT_LE( abs(test_poly[2]), eps );
  EXPECT_LE( abs(test_poly[3]), eps );
  cubic.get_poly_func_vals (test_poly, &test_coor[1] );
  EXPECT_LE( abs(test_poly[0]), eps );
  EXPECT_FLOAT_EQ( test_poly[1], 1.0f );
  EXPECT_LE( abs(test_poly[2]), eps );
  EXPECT_LE( abs(test_poly[3]), eps );
  cubic.get_poly_func_vals (test_poly, &test_coor[3] );
  EXPECT_LE( abs(test_poly[0]), eps );
  EXPECT_LE( abs(test_poly[1]), eps );
  EXPECT_FLOAT_EQ( test_poly[2], 1.0f );
  EXPECT_LE( abs(test_poly[3]), eps );
  cubic.get_poly_func_vals (test_poly, &test_coor[4] );
  EXPECT_LE( abs(test_poly[0]), eps );
  EXPECT_LE( abs(test_poly[1]), eps );
  EXPECT_LE( abs(test_poly[2]), eps );
  EXPECT_FLOAT_EQ( test_poly[3], 1.0f );
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
// Strain calculation tests for bar elements in 1D and 3D space
// J = dN1 * vert_coor, dN1 is P1 shape function
// G = inv(J) * dNp, inv here is a pseudo-inverse for 1D bar element
// strain = G * u
TEST( GridElemBars, StrainCalculation1D ){
  //NOTE this tests a prototype for all element strain calculations:
  //     J = dN1 * vert_coor, G = inv(J) * dNp, strain = G * u.
  //     This default implementation also gets the non-inverted J determinant.
  //
  const fmr::Dim_int sims_d = 1;
  // First, check P1 element
  const double vert_coor[2*sims_d] = {0.0, 2.0};
  const double u[2*sims_d] = {0.2, 0.4};
  constexpr fmr::Local_int intp_n = 2;
  const double intp1[intp_n] = {0.25, 0.75};
  double dN1[intp_n*2];
  double dNp[intp_n*2];
  const fmr::Local_int jacs_size = Elem<Bars>::jacs_size (intp_n, sims_d);
  const fmr::Local_int jacs_dets_size = Elem<Bars>::jacs_dets_size (intp_n, sims_d);
  double J[jacs_size];
  double jacs_dets[jacs_dets_size];
  double Jinv[jacs_size];
  double G[intp_n*2];
  double strain[intp_n];
  for(fmr::Local_int ip=0; ip<intp_n; ++ip){
    fmr::fems::elem::calc_poly_grad_node_C1D2N(dN1 + ip*2, intp1 + ip);
    fmr::fems::elem::calc_poly_grad_node_C1D2N(dNp + ip*2, intp1 + ip);
    EXPECT_DOUBLE_EQ(dN1[ip*2], -1.0);
    EXPECT_DOUBLE_EQ(dN1[ip*2+1], 1.0);

    J[ip] = 0.0;
    for(fmr::Local_int i=0; i<2; ++i){
      J[ip] += dN1[ip*2+i] * vert_coor[i];
    }
    fmr::math::smat_invt_det<1> (jacs_dets + ip*2, J + ip);
    Jinv[ip] = jacs_dets[ip*2];
    EXPECT_DOUBLE_EQ(Jinv[ip], 1.0 / J[ip]);
    EXPECT_DOUBLE_EQ(jacs_dets[ip*2+1], J[ip]);

    for(fmr::Local_int i=0; i<2; ++i){
      G[ip*2+i] = Jinv[ip] * dNp[ip*2+i];
    }
    strain[ip] = 0.0;
    for(fmr::Local_int i=0; i<2; ++i){
      strain[ip] += G[ip*2+i] * u[i];
    }
    EXPECT_DOUBLE_EQ(strain[ip], 0.1);
  }
}
TEST( GridElemBars, StrainCalculation3D ){
  const double vert_coor[6] = {1.0,2.0,3.0, 3.0,5.0,9.0};
  const double u[6] = {0.2,0.4,0.6, 0.4,0.8,1.2};
  constexpr fmr::Local_int intp_n = 2;
  const double intp1[intp_n] = {0.25, 0.75};
  double strain[intp_n];
  double dN1[intp_n*2];
  double dNp[intp_n*2];
  double J[intp_n*3];
  double Jpinv[intp_n*3];
  double G[intp_n*6];
  const double length_sq = 49.0;
  const double expected_strain
    = (0.2*2.0 + 0.4*3.0 + 0.6*6.0) / length_sq;

  for(fmr::Local_int ip=0; ip<intp_n; ++ip){
    fmr::fems::elem::calc_poly_grad_node_C1D2N(
      dN1 + ip*2, intp1 + ip);
    fmr::fems::elem::calc_poly_grad_node_C1D2N(
      dNp + ip*2, intp1 + ip);
    EXPECT_DOUBLE_EQ(dN1[ip*2], -1.0);
    EXPECT_DOUBLE_EQ(dN1[ip*2+1], 1.0);

    for(fmr::Dim_int dim=0; dim<3; ++dim){
      J[ip*3+dim] = 0.0;
      for(fmr::Local_int node=0; node<2; ++node){
        J[ip*3+dim] += dN1[ip*2+node] * vert_coor[node*3+dim];
      }
    }
    ASSERT_EQ(0, fmr::math::smat_pinv_col_ref<3>(
      Jpinv + ip*3, J + ip*3));

    for(fmr::Local_int node=0; node<2; ++node){
      for(fmr::Dim_int dim=0; dim<3; ++dim){
        G[ip*6+node*3+dim]
          = Jpinv[ip*3+dim] * dNp[ip*2+node];
      }
    }

    double displacement_gradient[9] = {};
    for(fmr::Local_int node=0; node<2; ++node){
      for(fmr::Dim_int disp_dim=0; disp_dim<3; ++disp_dim){
        for(fmr::Dim_int coor_dim=0; coor_dim<3; ++coor_dim){
          displacement_gradient[disp_dim*3+coor_dim]
            += u[node*3+disp_dim] * G[ip*6+node*3+coor_dim];
        }
      }
    }
    strain[ip] = 0.0;
    for(fmr::Dim_int disp_dim=0; disp_dim<3; ++disp_dim){
      for(fmr::Dim_int coor_dim=0; coor_dim<3; ++coor_dim){
        strain[ip] += J[ip*3+disp_dim] / 7.0
          * displacement_gradient[disp_dim*3+coor_dim]
          * J[ip*3+coor_dim] / 7.0;
      }
    }
    EXPECT_NEAR(strain[ip], expected_strain, 1e-14);
  }
}
#endif
} } } // end namespace femera::grid::elem
#undef DEBUG_PRINT
