#include "../../femera.hpp"
#include "../Elem.test.hpp"
#include "Tets.hpp"
#include "../../../fmr/fems/elem-poly.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

float tet_vol_ref = float (1.0 / 6.0);
fmr::Phys_float wsum1=0.0, wsum4=0.0, wsum5=0.0, wsum10=0.0, wsum11=0.0;

// Both of the following two seem to work for the defaults.
femera::grid::elem::Tets test_ctet;
femera::grid::elem::Elem<femera::grid::elem::Tets>   test_tet ;// P1 (default)
femera::grid::elem::Elem<femera::grid::elem::Tets,1> test_tet1;// P1 linear
femera::grid::elem::Elem<femera::grid::elem::Tets,2> test_tet2;// P2 quadratic
femera::grid::elem::Elem<femera::grid::elem::Tets,3> test_tet3;// P3 cubic
//
fmr::Geom_float coor1 [12];
int tet4_tranpose_bad=0, tet10_tranpose_bad=0, tet20_tranpose_bad=0;
//
fmr::Geom_float ntrl_jac3_trace_p1 = 0.0;
fmr::Geom_float ntrl_jac3_trace_p2 = 0.0;
fmr::Geom_float ntrl_jac3_trace_p3 = 0.0;
//

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  for (int i=0; i< 1; ++i) {
    wsum1  += femera::grid::elem::Tets::intg_1_ptwt  [4*i +3];}
  for (int i=0; i< 4; ++i) {
    wsum4  += femera::grid::elem::Tets::intg_4_ptwt  [4*i +3];}
  for (int i=0; i< 5; ++i) {
    wsum5  += femera::grid::elem::Tets::intg_5_ptwt  [4*i +3];}
  for (int i=0; i<10; ++i) {
    wsum10 += femera::grid::elem::Tets::intg_10_ptwt [4*i +3];}
  for (int i=0; i<11; ++i) {
    wsum11 += femera::grid::elem::Tets::intg_11_ptwt [4*i +3];}
  //
  //Check coordinate transpose correct
  for (int i=0; i<4; ++i){
    for (int j=0; j<3; ++j){ tet4_tranpose_bad
      += (std::abs(femera::grid::elem::Tets::vert_coor[3*i+j]
        - femera::grid::elem::Tets::coor_vert[4*j+i])
        < (10.0*double(std::numeric_limits<float>::epsilon()))) ? 0 : 1;
  } }
  for (int i=0; i<10; ++i){
    for (int j=0; j<3; ++j){ tet10_tranpose_bad
      += (std::abs(femera::grid::elem::Tets::tet10_coor[3*i+j]
        - femera::grid::elem::Tets::coor_tet10[10*j+i])
        < (10.0*double(std::numeric_limits<float>::epsilon()))) ? 0 : 1;
  } }
  for (int i=0; i<20; ++i){
    for (int j=0; j<3; ++j){ tet20_tranpose_bad
      += (std::abs(femera::grid::elem::Tets::tet20_coor[3*i+j]
        - femera::grid::elem::Tets::coor_tet20[20*j+i])
        < (10.0*double(std::numeric_limits<float>::epsilon()))) ? 0 : 1;
  } }
  //tet4_tranpose_bad
  //
  // Calculate the Jacbian of a natural tet, which should be the 3x3 identity,
  // and is constant, independent of the integration point coordinates.
  // The one-point tet integration rule is sufficient.
  fmr::Geom_float intp [ 3];
  for (int i=0; i<3;++i){intp[i] = femera::grid::elem::Tets::intg_1_ptwt[i];}
  //const auto intw = femera::grid::fems::tets_intg_1_ptwt [3];
  fmr::Geom_float shpg [12]
     = {0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};
  femera::grid::elem::Tets::calc_poly_grad_node_4 (&shpg[0], &intp[0]);
  fmr::Geom_float coor [12];
  for (int i=0; i<12; ++i){coor[i] = femera::grid::elem::Tets::coor_vert[i];}
  fmr::Geom_float jac3 [ 9] = {0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};
  //coor = femera::grid::fems::tets_vert_coor;// 4x3
  //coor = femera::grid::fems::tets_coor_vert;// 3x4 **Use this
  for (int i=0; i<3; ++i) {
    for (int j=0; j<3; ++j) {
      for (int k=0; k<4; ++k) {
        jac3 [3*i +j]+= shpg [4*i +k] * coor [4*j +k];
  } } }
  ntrl_jac3_trace_p1 = jac3[0] + jac3[4] + jac3[8];// 3 for 3x3 identity
  //
  // Test 10-node tet for correct node ordering.
  fmr::Geom_float shp2 [30];
  for (int i=0; i<30; ++i){shp2[i]=0.0;}
  for (int i=0; i< 9; ++i){jac3[i]=0.0;}
  fmr::Geom_float coo2 [30];
  for (int i=0; i<30; ++i){coo2[i] = femera::grid::elem::Tets::coor_tet10[i];}
  femera::grid::elem::Tets::calc_poly_grad_node_10 (&shp2[0], &intp[0]);
  for (int i=0; i<3; ++i) {
    for (int j=0; j<3; ++j) {
      for (int k=0; k<10; ++k) {
        jac3 [3*i +j]+= shp2 [10*i +k] * coo2 [10*j +k];
  } } }
  ntrl_jac3_trace_p2 = jac3[0] + jac3[4] + jac3[8];
  //
  // Test 20-node tet for correct node ordering.
  fmr::Geom_float shp3 [60];
  for (int i=0; i<60; ++i){shp3[i]=0.0;}
  for (int i=0; i< 9; ++i){jac3[i]=0.0;}
  fmr::Geom_float coo3 [60];
  for (int i=0; i<60; ++i){coo3[i] = femera::grid::elem::Tets::coor_tet20[i];}
  femera::grid::elem::Tets::calc_poly_grad_node_20 (&shp3[0], &intp[0]);
  for (int i=0; i<3; ++i) {
    for (int j=0; j<3; ++j) {
      for (int k=0; k<20; ++k) {
        jac3 [3*i +j]+= shp3 [20*i +k] * coo3 [20*j +k];
  } } }
  ntrl_jac3_trace_p3 = jac3[0] + jac3[4] + jac3[8];
  //
#if 1
  printf ("%s\n[%f, %f, %f\n %f, %f, %f\n %f, %f, %f]\n", "Tets.gtst.cpp",
    jac3[0],jac3[1],jac3[2],jac3[3],jac3[4],jac3[5],jac3[6],jac3[7],jac3[8] );
#endif
  //
  return mini.exit ();
}
//-----------------------------
TEST( GridElemTets, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
// Check stuff in element info.


//-----------------------------
#if 0
TEST( GridElemTets, CtetVolOneSixth ){
  EXPECT_FLOAT_EQ( float(test_ctet.get_elem_v ()), tet_vol_ref );
}
TEST( GridElemTets, EtetVolOneSixth ){
  EXPECT_FLOAT_EQ( float(test_tet.get_elem_v ()), tet_vol_ref );
}
TEST( GridElemTets, AnyVolOneSixth ){
  EXPECT_FLOAT_EQ( float(femera::grid::elem::Tets::elem_v), tet_vol_ref );
}
#endif
#if 0
TEST( GridCellTets, P1_Node_N ){
  EXPECT_EQ(//NOTE it looks like googletest can't parse this correctly.
    femera::grid::elem::Elem<femera::grid::elem::Tets,1>::get_node_n (), 4 );
}
#endif
//-----------------------------
TEST( GridElemTets, ElementOrderP ){
  EXPECT_EQ( test_tet.get_elem_p(), test_tet1.get_elem_p() );
  EXPECT_EQ( test_tet1.get_elem_p(), 1 );
  EXPECT_EQ( test_tet2.get_elem_p(), 2 );
  EXPECT_EQ( test_tet3.get_elem_p(), 3 );
}
//-----------------------------
TEST( GridElemTets, ElementNodeCount ){
  EXPECT_EQ( test_tet.get_node_n(), test_tet1.get_node_n() );// default linear tet
  EXPECT_EQ( test_tet1.get_node_n(),  4 );
  EXPECT_EQ( test_tet2.get_node_n(), 10 );
  EXPECT_EQ( test_tet3.get_node_n(), 20 );
}
TEST( GridElemTets, ElementPolynomialType ){// Node count equal to poly term count
  EXPECT_EQ( test_tet1.get_node_n(), fmr::math::poly_term_n(
    fmr::math::Poly::Full, test_tet1.get_elem_p(), test_tet1.get_elem_d()) );
  EXPECT_EQ( test_tet2.get_node_n(), fmr::math::poly_term_n(
    fmr::math::Poly::Full, test_tet2.get_elem_p(), test_tet2.get_elem_d()) );
  EXPECT_EQ( test_tet3.get_node_n(), fmr::math::poly_term_n(
    fmr::math::Poly::Full, test_tet3.get_elem_p(), test_tet3.get_elem_d()) );
}
//-----------------------------
TEST( GridElemTets, IntgWgtsSumVol1 ){
  EXPECT_FLOAT_EQ( float(wsum1), tet_vol_ref );
}
TEST( GridElemTets, IntgWgtsSumVol4 ){
  EXPECT_FLOAT_EQ( float(wsum4), tet_vol_ref );
}
TEST( GridElemTets, IntgWgtsSumVol5 ){
  EXPECT_FLOAT_EQ( float(wsum5), tet_vol_ref );
}
TEST( GridElemTets, IntgWgtsSumVol10 ){
  EXPECT_FLOAT_EQ( float(wsum10), tet_vol_ref );
}
TEST( GridElemTets, IntgWgtsSumVol11 ){
  EXPECT_FLOAT_EQ( float(wsum11), tet_vol_ref );
}
//-----------------------------
//NOTE Google test fails report undefined reference and don't link.
TEST( GridElemTets, ArrayAccessInline ){
  EXPECT_EQ( test_tet1.get_bars_vert_conn ()[0], 0 );
  EXPECT_EQ( test_tet1.get_bars_vert_conn ()[1], 1 );

  auto vert1 = test_tet1.get_bars_vert_conn ();
  EXPECT_EQ( vert1 [0], 0 );
  EXPECT_EQ( vert1 [1], 1 );
}
TEST( GridElemTet, JacTraceIsThree ){
  EXPECT_FLOAT_EQ( float (ntrl_jac3_trace_p1), float (3.0) );
  EXPECT_FLOAT_EQ( float (ntrl_jac3_trace_p2), float (3.0) );
  EXPECT_FLOAT_EQ( float (ntrl_jac3_trace_p3), float (3.0) );//TODO node order
}
TEST( GridElemTets, CoorTranspose ){
  EXPECT_EQ( tet4_tranpose_bad, 0 );
  EXPECT_EQ( tet10_tranpose_bad, 0 );
  EXPECT_EQ( tet20_tranpose_bad, 0 );
}
TEST( GridElemTets, NodeOrderingAndBasisAgreement ){
  using femera::grid::elem::Elem;
  using femera::grid::elem::Tets;
  double p2[30] = {}, p3[60] = {};
  ASSERT_TRUE((Elem<Tets,2>::get_elem_node_coor(p2)));
  ASSERT_TRUE((Elem<Tets,3>::get_elem_node_coor(p3)));
  for (int i=0; i<30; ++i) EXPECT_NEAR(p2[i], Tets::tet10_coor[i], 1e-14);
  for (int i=0; i<60; ++i) EXPECT_NEAR(p3[i], Tets::tet20_coor[i], 1e-14);
  for (int n=0; n<10; ++n) {
    double values[10] = {};
    Tets::calc_poly_func_10(values, p2+3*n);
    for (int i=0; i<10; ++i) EXPECT_NEAR(values[i], i==n ? 1.0 : 0.0, 1e-13);
  }
  for (int n=0; n<20; ++n) {
    double values[20] = {};
    Tets::calc_poly_func_20(values, p3+3*n);
    for (int i=0; i<20; ++i) EXPECT_NEAR(values[i], i==n ? 1.0 : 0.0, 1e-13);
  }
  const double points[2][3] = {{0.13,0.21,0.17}, {0.4,0.1,0.2}};
  for (const auto& x : points) {
    double g2[30] = {}, g3[60] = {};
    Tets::calc_poly_grad_node_10(g2, x);
    Tets::calc_poly_grad_node_20(g3, x);
    for (int d=0; d<3; ++d) {
      double plus[3] = {x[0],x[1],x[2]}, minus[3] = {x[0],x[1],x[2]};
      plus[d] += 1e-6;
      minus[d] -= 1e-6;
      double f2p[10] = {}, f2m[10] = {}, f3p[20] = {}, f3m[20] = {};
      Tets::calc_poly_func_10(f2p, plus);
      Tets::calc_poly_func_10(f2m, minus);
      Tets::calc_poly_func_20(f3p, plus);
      Tets::calc_poly_func_20(f3m, minus);
      for (int i=0; i<10; ++i) EXPECT_NEAR(g2[d*10+i], (f2p[i]-f2m[i])/2e-6, 1e-8);
      for (int i=0; i<20; ++i) EXPECT_NEAR(g3[d*20+i], (f3p[i]-f3m[i])/2e-6, 1e-8);
    }
  }
}
TEST( GridElemTets, CubicTriangularFaceTraces ){
  using femera::grid::elem::Tets;
  for (int face=0; face<4; ++face) {
    const auto* vertices = Tets::tris_vert_conn+3*face;
    int mapping[10] = {int(vertices[0]), int(vertices[1]), int(vertices[2])};
    for (int edge=0; edge<3; ++edge) {
      const auto a = vertices[edge], b = vertices[(edge+1)%3];
      int matched = -1;
      bool reversed = false;
      for (int e=0; e<6; ++e) {
        const auto start = Tets::bars_vert_conn[2*e];
        const auto end = Tets::bars_vert_conn[2*e+1];
        if ((start == a && end == b) || (start == b && end == a)) {
          matched = e;
          reversed = start != a;
        }
      }
      ASSERT_GE(matched, 0);
      mapping[3+2*edge] = 4+2*matched+(reversed ? 1 : 0);
      mapping[4+2*edge] = 4+2*matched+(reversed ? 0 : 1);
    }
    mapping[9] = 16+face;
    const double points[2][3] = {{0.13,0.27,0}, {0.52,0.21,0}};
    for (const auto& uv : points) {
      double x[3] = {}, tangent[2][3] = {};
      for (uint d=0; d<3; ++d) {
        const double a = Tets::vert_coor[3*vertices[0]+d];
        tangent[0][d] = Tets::vert_coor[3*vertices[1]+d]-a;
        tangent[1][d] = Tets::vert_coor[3*vertices[2]+d]-a;
        x[d] = a+uv[0]*tangent[0][d]+uv[1]*tangent[1][d];
      }
      double tri[10] = {}, tri_grad[20] = {}, tet[20] = {}, tet_grad[60] = {};
      fmr::fems::elem::calc_poly_func_T2D10N(tri, uv);
      fmr::fems::elem::calc_poly_grad_node_T2D10N(tri_grad, uv);
      Tets::calc_poly_func_20(tet, x);
      Tets::calc_poly_grad_node_20(tet_grad, x);
      for (int i=0; i<10; ++i) {
        EXPECT_NEAR(tri[i], tet[mapping[i]], 1e-13);
        for (int d=0; d<2; ++d) {
          double derivative = 0;
          for (int k=0; k<3; ++k) derivative += tet_grad[20*k+mapping[i]]*tangent[d][k];
          EXPECT_NEAR(tri_grad[2*i+d], derivative, 1e-13);
        }
      }
      for (int i=0; i<20; ++i) {
        bool on_face = false;
        for (int j=0; j<10; ++j) if (mapping[j] == i) on_face = true;
        if (!on_face) {
          EXPECT_NEAR(tet[i], 0, 1e-13);
          for (int d=0; d<2; ++d) {
            double derivative = 0;
            for (int k=0; k<3; ++k) derivative += tet_grad[20*k+i]*tangent[d][k];
            EXPECT_NEAR(derivative, 0, 1e-13);
          }
        }
      }
    }
  }
}
namespace femera { namespace grid { namespace elem {

TEST( GridCellElemTet, FaceNormalVerts ){
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* 0],
    &Tets::vert_coor [3* 1],
    &Tets::vert_coor [3* 2]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* 2],
    &Tets::vert_coor [3* 1],
    &Tets::vert_coor [3* 0]), "-z" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* 0],
    &Tets::vert_coor [3* 3],
    &Tets::vert_coor [3* 1]), "+y" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* 0],
    &Tets::vert_coor [3* 2],
    &Tets::vert_coor [3* 3]), "+x" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* 1],
    &Tets::vert_coor [3* 3],
    &Tets::vert_coor [3* 2]),
    "[-0.577350, -0.577350, -0.577350]" );// -sqrt(1/3)
}
TEST( GridCellElemTet, FaceNormalTris ){
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* Tets::tris_vert_conn [0]],
    &Tets::vert_coor [3* Tets::tris_vert_conn [1]],
    &Tets::vert_coor [3* Tets::tris_vert_conn [2]]), "-z" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* Tets::tris_vert_conn [3]],
    &Tets::vert_coor [3* Tets::tris_vert_conn [4]],
    &Tets::vert_coor [3* Tets::tris_vert_conn [5]]), "-y" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* Tets::tris_vert_conn [6]],
    &Tets::vert_coor [3* Tets::tris_vert_conn [7]],
    &Tets::vert_coor [3* Tets::tris_vert_conn [8]]), "-x" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* Tets::tris_vert_conn [ 9]],
    &Tets::vert_coor [3* Tets::tris_vert_conn [10]],
    &Tets::vert_coor [3* Tets::tris_vert_conn [11]]),
    "[0.577350, 0.577350, 0.577350]" );
}

} } }//end femera::grid::elem:: namespace
