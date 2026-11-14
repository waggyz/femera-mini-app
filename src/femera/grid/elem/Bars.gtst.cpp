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
  EXPECT_EQ( Elem<Bars>::get_poly_grad_size (), 6 );
  EXPECT_EQ( Elem<Bars>::get_poly_type (), fmr::math::Poly::Serendipity );
  
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
  const double xi = 0.5;// int point for Gauss Legendre first order quadrature
  double N_storage[2];
  double dN_storage[2 * 3];// Default 3D space
  double* N = N_storage;
  double* dN = dN_storage;
  fmr::fems::elem::calc_poly_func_C1D2N (N, &xi);
  fmr::fems::elem::calc_poly_grad_node_C1D2N (dN, &xi);
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
  constexpr Bars bars;
  const double xi = 0.5;
  double dN_storage[2 * 3];
  double* dN = dN_storage;
  double dNT_storage[2 * 3];
  double* dNT = dNT_storage;
  fmr::fems::elem::calc_poly_grad_node_C1D2N (dN, &xi);
  fmr::fems::elem::calc_poly_node_grad_C1D2N (dNT, &xi);
  fmr::Phys_float jacs_node_coor_storage[3 * 3];
  fmr::Phys_float* jacs_caos = jacs_node_coor_storage;
  fmr::Phys_float jacs_node_coor_a_storage[3 * 3];
  fmr::Phys_float* jacs_node_coor_a = jacs_node_coor_a_storage;
  fmr::Phys_float jacs_coor_node_storage[3 * 3];
  fmr::Phys_float* jacs_csoa = jacs_coor_node_storage;
  const double* node_coor = bars.get_elem_vert_coor ();// naos
  const double* coor_node = bars.get_elem_coor_vert ();// nsoa
  //
  fmr::fems::elem::elem_jacs_node_coor_ref (jacs_caos, node_coor, dNT, 1u, 2u, 3u);// coor aos
  fmr::fems::elem::elem_jacs_node_coor_a   (jacs_node_coor_a, node_coor, dN, 1u, 2u, 3u);// coor aos
  fmr::fems::elem::elem_jacs_coor_node_ref (jacs_csoa, coor_node, dNT, 1u, 2u, 3u);// coor soa
  //
  #ifdef DEBUG_PRINT
  std::cout << "jacs_caos:" << std::endl;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      std::cout  << jacs_caos[i * 3 + j] << " ";
    }
    std::cout << std::endl;
  }
  #endif
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
  EXPECT_DOUBLE_EQ( jacs_node_coor_a[0], jacs_caos[0] );
  EXPECT_DOUBLE_EQ( jacs_node_coor_a[1], jacs_caos[1] );
  EXPECT_DOUBLE_EQ( jacs_node_coor_a[2], jacs_caos[2] );
  EXPECT_DOUBLE_EQ( jacs_node_coor_a[3], jacs_caos[3] );
  EXPECT_DOUBLE_EQ( jacs_node_coor_a[4], jacs_caos[4] );
  EXPECT_DOUBLE_EQ( jacs_node_coor_a[5], jacs_caos[5] );
  EXPECT_DOUBLE_EQ( jacs_node_coor_a[6], jacs_caos[6] );
  EXPECT_DOUBLE_EQ( jacs_node_coor_a[7], jacs_caos[7] );
  EXPECT_DOUBLE_EQ( jacs_node_coor_a[8], jacs_caos[8] );
}
#endif
#if 1
TEST( GridElemBars, JacobianCalculationMultiplePointsAndDimensions ){
  constexpr Elem<Bars> bars;
  constexpr fmr::Local_int vert_n = bars.get_vert_n ();
  constexpr fmr::Local_int intp_n = 2;
  const double origin[3] = {2.0, 3.0, 5.0};
  const double edge[3] = {5.0, 8.0, 11.0};
  for(fmr::Dim_int dims=1; dims<=3; ++dims){
    SCOPED_TRACE(dims);
    double vert_coor[2 * 3];
    double coor_vert[2 * 3];
    double poly_grad[2 * 2 * 3];
    double poly_grat[2 * 2 * 3];
    fmr::Phys_float jacs_caos[2 * 3 * 3];
    fmr::Phys_float jacs_node_coor_a[2 * 3 * 3];
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
    fmr::fems::elem::elem_jacs_node_coor_ref (
      jacs_caos, vert_coor, poly_grad, intp_n, vert_n, dims);
    fmr::fems::elem::elem_jacs_node_coor_a (
      jacs_node_coor_a, vert_coor, poly_grat, intp_n, vert_n, dims);
    fmr::fems::elem::elem_jacs_coor_node_ref (
      jacs_csoa, coor_vert, poly_grad, intp_n, vert_n, dims);
    for(fmr::Local_int ip=0; ip<intp_n; ++ip){
      for(fmr::Local_int i=0; i<dims; ++i){
        for(fmr::Local_int j=0; j<dims; ++j){
          const auto index = ip*dims*dims + i*dims + j;
          const double expected = edge[i] * (ip*dims + j + 1);
          EXPECT_DOUBLE_EQ( jacs_caos[index], expected );
          EXPECT_DOUBLE_EQ( jacs_node_coor_a[index], expected );
          EXPECT_DOUBLE_EQ( jacs_csoa[index], expected );
        }
      }
    }
  }
}
#endif
} } } // end namespace femera::grid::elem
#undef DEBUG_PRINT