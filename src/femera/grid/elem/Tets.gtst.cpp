#include "../../femera.hpp"

#include "Tets.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

float tet_vol_ref = float (1.0 / 6.0);

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}

TEST( GridElemTets, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
// Check stuff in element info.
// Both of the following seem to work.
femera::grid::elem::Tets test_ctet;
femera::grid::elem::Elem<femera::grid::elem::Tets>   test_tet ;// P1 (default)
femera::grid::elem::Elem<femera::grid::elem::Tets,1> test_tet1;// P1 linear
femera::grid::elem::Elem<femera::grid::elem::Tets,2> test_tet2;// P2 quadratic
femera::grid::elem::Elem<femera::grid::elem::Tets,3> test_tet3;// P3 cubic

TEST( GridElemTets, CtetVolOneSixth ){
  EXPECT_FLOAT_EQ( float(test_ctet.get_elem_v ()), tet_vol_ref );
}
TEST( GridElemTets, EtetVolOneSixth ){
  EXPECT_FLOAT_EQ( float(test_tet.get_elem_v ()), tet_vol_ref );
}
TEST( GridElemTets, AtetVolOneSixth ){
  EXPECT_FLOAT_EQ( float(femera::grid::elem::Tets::elem_v), tet_vol_ref );
}

TEST( GridElemTets, DefaultTetOrder1 ){
  EXPECT_EQ( test_tet.get_elem_p(), 1 );
}
TEST( GridElemTets, P1TetTetOrder1 ){
  EXPECT_EQ( test_tet1.get_elem_p(), 1 );
}
TEST( GridElemTets, P2TetTetOrder2 ){
  EXPECT_EQ( test_tet2.get_elem_p(), 2 );
}
TEST( GridElemTets, P3TetTetOrder3 ){
  EXPECT_EQ( test_tet3.get_elem_p(), 3 );
}
TEST( GridElemTets, DefaultTetHas4Nodes ){
  EXPECT_EQ( test_tet.get_node_n(), 4 );
}
TEST( GridElemTets, P1TetHas4Nodes ){
  EXPECT_EQ( test_tet1.get_node_n(), 4 );
}
TEST( GridElemTets, P2TetHas10Nodes ){
  EXPECT_EQ( test_tet2.get_node_n(), 10 );
}
TEST( GridElemTets, P3TetHas20Nodes ){
  EXPECT_EQ( test_tet3.get_node_n(), 20 );
}

