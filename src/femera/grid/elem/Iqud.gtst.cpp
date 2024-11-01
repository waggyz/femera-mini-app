#include "../../femera.hpp"
#include "Iqud.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}

TEST( GridElemWdge, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

namespace femera { namespace grid { namespace elem {

TEST( GridCellFEmsFac4, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (// bottom face
    &Iqud::vert_coor [3* Iqud::quad_conn [0]],
    &Iqud::vert_coor [3* Iqud::quad_conn [1]],
    &Iqud::vert_coor [3* Iqud::quad_conn [2]]), "+z" );
  EXPECT_EQ( tri3_norm_str (// bottom face
    &Iqud::vert_coor [3* Iqud::quad_conn [2]],
    &Iqud::vert_coor [3* Iqud::quad_conn [3]],
    &Iqud::vert_coor [3* Iqud::quad_conn [0]]), "+z" );
  EXPECT_EQ( tri3_norm_str (// top face
    &Iqud::vert_coor [3* Iqud::quad_conn [4]],
    &Iqud::vert_coor [3* Iqud::quad_conn [5]],
    &Iqud::vert_coor [3* Iqud::quad_conn [6]]), "-z" );
  EXPECT_EQ( tri3_norm_str (// top face
    &Iqud::vert_coor [3* Iqud::quad_conn [6]],
    &Iqud::vert_coor [3* Iqud::quad_conn [7]],
    &Iqud::vert_coor [3* Iqud::quad_conn [4]]), "-z" );
}

} } }//end femera::grid::elem:: namespace
