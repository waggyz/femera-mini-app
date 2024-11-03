#include "../../femera.hpp"

#include "Tets.hpp"

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
fmr::Geom_float coor1 [12];
fmr::Geom_float ntrl_jac3_trace_p1 = 0.0;
fmr::Geom_float ntrl_jac3_trace_p2 = 0.0;

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
  // Calculate the Jacbian of a natural tet, which should be the 3x3 identity,
  // and is constant, independent of the integration point coordinates.
  // The one-point tet integration rule is sufficient.
  fmr::Geom_float intp [ 3];
  for (int i=0; i<3;++i){intp[i] = femera::grid::elem::Tets::intg_1_ptwt[i];}
  //const auto intw = femera::grid::fems::tets_intg_1_ptwt [3];
  fmr::Geom_float shpg [12]
     = {0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};
  femera::grid::elem::Tets::shap_grad_4 (&shpg[0], &intp[0]);
  fmr::Geom_float coor [12];
  for (int i=0; i<12;++i){coor[i] = femera::grid::elem::Tets::coor_vert[i];}
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
  fmr::Geom_float shp2 [30];
  for (int i=0; i<30; ++i){shp2[i]=0.0;}
  for (int i=0; i< 9; ++i){jac3[i]=0.0;}
  fmr::Geom_float coo2 [30];
  for (int i=0; i<30;++i){coo2[i] = femera::grid::elem::Tets::coor_node_2[i];}
  femera::grid::elem::Tets::shap_grad_10 (&shp2[0], &intp[0]);
  for (int i=0; i<3; ++i) {
    for (int j=0; j<3; ++j) {
      for (int k=0; k<10; ++k) {
        jac3 [3*i +j]+= shp2 [10*i +k] * coo2 [10*j +k];
  } } }
  ntrl_jac3_trace_p2 = jac3[0] + jac3[4] + jac3[8];
#if 0
  printf ("[%f, %f, %f,\n %f, %f, %f,\n %f, %f, %f]\n",
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
TEST( GridElemTets, DefaultTetOrder1 ){
  EXPECT_EQ( test_tet.get_elem_p(), 1 );
}
TEST( GridElemTets, P1Order1 ){
  EXPECT_EQ( test_tet1.get_elem_p(), 1 );
}
TEST( GridElemTets, P2Order2 ){
  EXPECT_EQ( test_tet2.get_elem_p(), 2 );
}
TEST( GridElemTets, P3Order3 ){
  EXPECT_EQ( test_tet3.get_elem_p(), 3 );
}
//-----------------------------
TEST( GridElemTets, DefaultHas4Nodes ){
  EXPECT_EQ( test_tet.get_node_n(), 4 );
}
TEST( GridElemTets, P1Has4Nodes ){
  EXPECT_EQ( test_tet1.get_node_n(), 4 );
}
TEST( GridElemTets, P2Has10Nodes ){
  EXPECT_EQ( test_tet2.get_node_n(), 10 );
}
TEST( GridElemTets, P3Has20Nodes ){
  EXPECT_EQ( test_tet3.get_node_n(), 20 );
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
//NOTE Google test fails report undefined reference and don't ;onk.
TEST( GridElemTets, ArrayAccessInline ){
  EXPECT_EQ( test_tet1.get_spar_conn ()[0], 0 );
  EXPECT_EQ( test_tet1.get_spar_conn ()[1], 1 );
}
auto vert1 = test_tet1.get_spar_conn ();
//auto s0 = &spar1[0];
auto v0 = &test_tet1.get_vert_conn ()[0];
auto v1 = &test_tet1.get_vert_conn ()[1];
TEST( GridElemTets, ArrayAccessVariable ){
  EXPECT_EQ( vert1 [0], 0 );
  EXPECT_EQ( vert1 [1], 1 );
  EXPECT_EQ( v0 [0], 0 );
  EXPECT_EQ( v0 [1], 1 );
  EXPECT_EQ( v1 [0], 1 );
  EXPECT_EQ( v1 [1], 2 );
}
TEST( GridElemTet, JacTraceThree ){
  EXPECT_FLOAT_EQ( float(ntrl_jac3_trace_p1), float (3.0) );
  EXPECT_FLOAT_EQ( float(ntrl_jac3_trace_p2), float (3.0) );
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
    &Tets::vert_coor [3* Tets::tris_conn [0]],
    &Tets::vert_coor [3* Tets::tris_conn [1]],
    &Tets::vert_coor [3* Tets::tris_conn [2]]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* Tets::tris_conn [3]],
    &Tets::vert_coor [3* Tets::tris_conn [4]],
    &Tets::vert_coor [3* Tets::tris_conn [5]]), "+y" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* Tets::tris_conn [6]],
    &Tets::vert_coor [3* Tets::tris_conn [7]],
    &Tets::vert_coor [3* Tets::tris_conn [8]]), "+x" );
  EXPECT_EQ( tri3_norm_str (
    &Tets::vert_coor [3* Tets::tris_conn [ 9]],
    &Tets::vert_coor [3* Tets::tris_conn [10]],
    &Tets::vert_coor [3* Tets::tris_conn [11]]),
    "[-0.577350, -0.577350, -0.577350]" );
}

} } }//end femera::grid::elem:: namespace
