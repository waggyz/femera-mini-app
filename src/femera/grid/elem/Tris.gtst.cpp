#include "../../femera.hpp"
#include "Tris.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
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

} } }//end femera::grid::elem:: namespace
