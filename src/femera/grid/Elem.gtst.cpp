#include "../femera.hpp"

#include "elem/Tets.hpp"
#include "elem/Prmd.hpp"
#include "elem/Wdge.hpp"
#include "elem/Itri.hpp"
#include "elem/Iqud.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Local_int jacs_size=0, jac1_size=0, jac2_size=0, jac3_size=0, jac4_size=0;
fmr::Local_int elem_tet2_node_n=0;

constexpr auto elem_tet1 = femera::grid::elem::Elem<femera::grid::elem::Tets,1> ();
constexpr auto elem_tet2 = femera::grid::elem::Elem<femera::grid::elem::Tets,2> ();
constexpr auto elem_tet3 = femera::grid::elem::Elem<femera::grid::elem::Tets,3> ();

// Supported element examples
static constexpr fmr::grid::elem::Elem_args tri6_2d_args = { 6, 3, 2};
static constexpr fmr::grid::elem::Elem_args tri6_3d_args = { 6, 3, 3};
static constexpr fmr::grid::elem::Elem_args tet10_args   = {10, 4, 3};

uintptr_t val0_adress=0, val1_adress=0;

constexpr auto test_tet_3d = fmr::grid::elem::Elem_test
  <femera::grid::elem::Tets, tet10_args> ();

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  const auto elem_quad = femera::grid::elem::Elem<femera::grid::elem::Quad> ();
  const auto elem_spar = femera::grid::elem::Elem<femera::grid::elem::Spar,1,1> ();
  const auto elem_tris = femera::grid::elem::Elem<femera::grid::elem::Tris,1,2> ();
  //
  jacs_size = elem_quad.jacd_size ();
  jac1_size = elem_spar.jacd_size ();
  jac2_size = elem_tris.jacd_size ();
  jac3_size = elem_tet1.jacd_size (1);
  jac4_size = elem_tet1.jacd_size (4);
  //
  elem_tet2_node_n = elem_tet2.get_node_n ();
  //
#pragma omp parallel
  {// Check that variables assigned from constexpr values are thread-local.
    auto elem = femera::grid::elem::Elem<femera::grid::elem::Tets,2> ();
    const auto val_local = elem.get_node_n ();
    if (omp_get_thread_num () == 0){
      val0_adress = reinterpret_cast<uintptr_t>(&val_local);
    } 
    if (omp_get_thread_num () == 1){
      val1_adress = reinterpret_cast<uintptr_t>(&val_local);
    }
  }//end parallel region
  //
  return mini.exit ();
}
TEST( GridElem, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
TEST( GridElem, JacdSizeTest ){
  EXPECT_EQ( jacs_size, 10 );// 3D, 1 point rule default
  EXPECT_EQ( jac1_size,  2 );
  EXPECT_EQ( jac2_size,  5 );
  EXPECT_EQ( jac3_size, 10 );
  EXPECT_EQ( jac4_size, 40 );// 3D, 4 points
}
TEST( GridElem, TetsVertN ){
  EXPECT_EQ( elem_tet1.get_vert_n (), 4 );
}
TEST( GridElem, FaceN ){
  EXPECT_EQ( elem_tet1.get_face_n (), 4 );
}
TEST( GridElem, TetsConnN ){
  EXPECT_EQ( elem_tet1.get_node_n (), 4 );
  EXPECT_EQ( elem_tet2.get_node_n (),10 );
  EXPECT_EQ( elem_tet3.get_node_n (),20 );
  EXPECT_EQ( elem_tet2_node_n       ,10 );
}
TEST( GridElem, Measures ){
  EXPECT_FLOAT_EQ( float(elem_tet1.get_elem_v ()), float(1.0/6.0) );
  EXPECT_FLOAT_EQ( float(elem_tet1.get_elem_d ()), 3 );
}
TEST( GridElem, ShapeGradient ){
  EXPECT_EQ( elem_tet1.shpg_size ( ), 12 );
  EXPECT_EQ( elem_tet2.shpg_size ( ), 30 );
  EXPECT_EQ( elem_tet3.shpg_size ( ), 60 );
  EXPECT_EQ( elem_tet2.shpg_size (4),120 );
}
TEST( ElemStruct, Works ){
  EXPECT_EQ( test_tet_3d.test_vert_n (), 4);
}
TEST( ElemStruct, ThreadLocalAssignedFromConstexpr ){
  EXPECT_NE( val0_adress, val1_adress);
}

