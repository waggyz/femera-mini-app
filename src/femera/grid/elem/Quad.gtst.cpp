#include "../../femera.hpp"
#include "Quad.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}

TEST( GridElemQuad, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

namespace femera { namespace grid { namespace elem {

TEST( GridCellFEmsQuad, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (
    &Quad::vert_coor [3* 0],
    &Quad::vert_coor [3* 1],
    &Quad::vert_coor [3* 2]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &Quad::vert_coor [3* 2],
    &Quad::vert_coor [3* 3],
    &Quad::vert_coor [3* 0]), "+z" );
}

} } }//end femera::grid::elem:: namespace
