#include "../femera.hpp"
#include "Elem.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Local_int jacs_size=0, jac1_size=0, jac2_size=0, jac3_size=0, jac4_size=0;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  jacs_size
    = femera::grid::elem::Elem<femera::grid::elem::Quad>::jacd_size ();
  jac1_size
    = femera::grid::elem::Elem<femera::grid::elem::Spar,1,1>::jacd_size ();
  jac2_size
    = femera::grid::elem::Elem<femera::grid::elem::Tris,1,2>::jacd_size ();
  jac3_size
    = femera::grid::elem::Elem<femera::grid::elem::Tets,1,3>::jacd_size (1);
  jac4_size
    = femera::grid::elem::Elem<femera::grid::elem::Tets,1,3>::jacd_size (4);
  //
  return mini.exit ();
}

TEST( GridElem, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
TEST( GridElem, JacdSizeTest ){
  EXPECT_EQ( jacs_size, 10 );
  EXPECT_EQ( jac1_size,  2 );
  EXPECT_EQ( jac2_size,  5 );
  EXPECT_EQ( jac3_size, 10 );
  EXPECT_EQ( jac4_size, 40 );
}

