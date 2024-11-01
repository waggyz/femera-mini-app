#include "../../femera.hpp"
#include "Itri.hpp"

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

TEST( GridCellFEmsFac3, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (// bottom face
    &Itri::vert_coor [3* Itri::tris_conn [0]],
    &Itri::vert_coor [3* Itri::tris_conn [1]],
    &Itri::vert_coor [3* Itri::tris_conn [2]]), "+z" );
  EXPECT_EQ( tri3_norm_str (// top face
    &Itri::vert_coor [3* Itri::tris_conn [3]],
    &Itri::vert_coor [3* Itri::tris_conn [4]],
    &Itri::vert_coor [3* Itri::tris_conn [5]]), "-z" );
}

} } }//end femera::grid::elem:: namespace
