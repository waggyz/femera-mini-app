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

auto elem_tet1 = femera::grid::elem::Elem<femera::grid::elem::Tets,1> ();
auto elem_tet2 = femera::grid::elem::Elem<femera::grid::elem::Tets,2> ();
auto elem_tet3 = femera::grid::elem::Elem<femera::grid::elem::Tets,3> ();

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  auto elem_quad = femera::grid::elem::Elem<femera::grid::elem::Quad> ();
  auto elem_spar = femera::grid::elem::Elem<femera::grid::elem::Spar,1,1> ();
  auto elem_tris = femera::grid::elem::Elem<femera::grid::elem::Tris,1,2> ();
  //
  jacs_size = elem_quad.jacd_size ();
  jac1_size = elem_spar.jacd_size ();
  jac2_size = elem_tris.jacd_size ();
  jac3_size = elem_tet1.jacd_size (1);
  jac4_size = elem_tet1.jacd_size (4);
  //
  elem_tet2_node_n = elem_tet2.get_node_n ();
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

