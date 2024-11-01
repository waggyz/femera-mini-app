#include "../../femera.hpp"
#include "Cube.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}

TEST( GridElemCube, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

namespace femera { namespace grid { namespace elem {

// Check Cube element surfaces as triangles
TEST( GridCellFEmsCube, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (// bottom face
    & Cube::vert_coor [3*  Cube::quad_conn [0]],
    & Cube::vert_coor [3*  Cube::quad_conn [1]],
    & Cube::vert_coor [3*  Cube::quad_conn [2]]), "+z" );
  EXPECT_EQ( tri3_norm_str (// bottom face
    & Cube::vert_coor [3*  Cube::quad_conn [2]],
    & Cube::vert_coor [3*  Cube::quad_conn [3]],
    & Cube::vert_coor [3*  Cube::quad_conn [0]]), "+z" );
  EXPECT_EQ( tri3_norm_str (// top face
    & Cube::vert_coor [3*  Cube::quad_conn [4]],
    & Cube::vert_coor [3*  Cube::quad_conn [5]],
    & Cube::vert_coor [3*  Cube::quad_conn [6]]), "-z" );
  EXPECT_EQ( tri3_norm_str (// top face
    & Cube::vert_coor [3*  Cube::quad_conn [6]],
    & Cube::vert_coor [3*  Cube::quad_conn [7]],
    & Cube::vert_coor [3*  Cube::quad_conn [4]]), "-z" );
  EXPECT_EQ( tri3_norm_str (
    & Cube::vert_coor [3*  Cube::quad_conn [ 8]],
    & Cube::vert_coor [3*  Cube::quad_conn [ 9]],
    & Cube::vert_coor [3*  Cube::quad_conn [10]]), "+y" );
  EXPECT_EQ( tri3_norm_str (
    & Cube::vert_coor [3*  Cube::quad_conn [10]],
    & Cube::vert_coor [3*  Cube::quad_conn [11]],
    & Cube::vert_coor [3*  Cube::quad_conn [ 8]]), "+y" );
  EXPECT_EQ( tri3_norm_str (
    & Cube::vert_coor [3*  Cube::quad_conn [12]],
    & Cube::vert_coor [3*  Cube::quad_conn [13]],
    & Cube::vert_coor [3*  Cube::quad_conn [14]]), "-x" );
  EXPECT_EQ( tri3_norm_str (
    & Cube::vert_coor [3*  Cube::quad_conn [14]],
    & Cube::vert_coor [3*  Cube::quad_conn [15]],
    & Cube::vert_coor [3*  Cube::quad_conn [12]]), "-x" );
  EXPECT_EQ( tri3_norm_str (
    & Cube::vert_coor [3*  Cube::quad_conn [16]],
    & Cube::vert_coor [3*  Cube::quad_conn [17]],
    & Cube::vert_coor [3*  Cube::quad_conn [18]]), "-y" );
  EXPECT_EQ( tri3_norm_str (
    & Cube::vert_coor [3*  Cube::quad_conn [18]],
    & Cube::vert_coor [3*  Cube::quad_conn [19]],
    & Cube::vert_coor [3*  Cube::quad_conn [16]]), "-y" );
  EXPECT_EQ( tri3_norm_str (
    & Cube::vert_coor [3*  Cube::quad_conn [20]],
    & Cube::vert_coor [3*  Cube::quad_conn [21]],
    & Cube::vert_coor [3*  Cube::quad_conn [22]]), "+x" );
  EXPECT_EQ( tri3_norm_str (
    & Cube::vert_coor [3*  Cube::quad_conn [22]],
    & Cube::vert_coor [3*  Cube::quad_conn [23]],
    & Cube::vert_coor [3*  Cube::quad_conn [20]]), "+x" );
}

} } }//end femera::grid::elem:: namespace
