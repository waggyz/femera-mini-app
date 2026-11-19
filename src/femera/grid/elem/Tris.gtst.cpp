#include "../../femera.hpp"
#include "../Elem.test.hpp"
#include "Tris.hpp"

#include <gtest/gtest.h>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}

TEST( GridElemTets, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

namespace femera { namespace grid { namespace elem {

TEST( GridCellFEmsTris, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (
    &Tris::vert_coor [3* 0],
    &Tris::vert_coor [3* 1],
    &Tris::vert_coor [3* 2]), "+z" );
}

TEST( GridCellFEmsTris, CubicNodeCoordinates ){
  using Cubic = Elem<Tris,3>;
  const double third = 1.0/3.0;
  const double expected[30] = {
    0,0,0, 1,0,0, 0,1,0,
    third,0,0, 2*third,0,0,
    2*third,third,0, third,2*third,0,
    0,2*third,0, 0,third,0,
    third,third,0
  };
  double storage[32] = {};
  storage[0] = 123.0;
  storage[31] = 456.0;
  ASSERT_EQ(Cubic::get_node_n(), 10);
  ASSERT_TRUE(Cubic::get_elem_node_coor(storage+1));
  EXPECT_DOUBLE_EQ(storage[0], 123.0);
  EXPECT_DOUBLE_EQ(storage[31], 456.0);
  for (int i=0; i<30; ++i) EXPECT_NEAR(storage[i+1], expected[i], 1e-14);
}

} } }//end femera::grid::elem:: namespace
