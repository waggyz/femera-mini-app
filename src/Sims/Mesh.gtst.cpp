#include "gtest/gtest.h"
#include "../base.h"

using namespace Femera;
#if 0
TEST( Mesh, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
#endif

TEST( MeshElemNodes, Bars ){//TODO Move to math.gtest.cpp
  EXPECT_EQ( 2, fmr::math::poly_terms (1, fmr::math::Poly::Full, 1) );
  EXPECT_EQ( 3, fmr::math::poly_terms (1, fmr::math::Poly::Full, 2) );
  EXPECT_EQ( 4, fmr::math::poly_terms (1, fmr::math::Poly::Full, 3) );
}
TEST( MeshElemNodes, Tris ){
  EXPECT_EQ( 3, fmr::math::poly_terms (2, fmr::math::Poly::Full, 1) );
  EXPECT_EQ( 6, fmr::math::poly_terms (2, fmr::math::Poly::Full, 2) );
  EXPECT_EQ(10, fmr::math::poly_terms (2, fmr::math::Poly::Full, 3) );
}
TEST( MeshElemNodes, Quad ){
  EXPECT_EQ( 4, fmr::math::poly_terms (2, fmr::math::Poly::Bipoly, 1) );
  EXPECT_EQ( 8, fmr::math::poly_terms (2, fmr::math::Poly::Serendipity, 2) );
  EXPECT_EQ(16, fmr::math::poly_terms (2, fmr::math::Poly::Bipoly, 3) );
}
TEST( MeshElemNodes, Tets ){
  EXPECT_EQ( 4, fmr::math::poly_terms (3, fmr::math::Poly::Full, 1) );
  EXPECT_EQ(10, fmr::math::poly_terms (3, fmr::math::Poly::Full, 2) );
  EXPECT_EQ(20, fmr::math::poly_terms (3, fmr::math::Poly::Full, 3) );
}
TEST( MeshElemNodes, Cube ){
  EXPECT_EQ( 8, fmr::math::poly_terms (3, fmr::math::Poly::Bipoly, 1) );
  EXPECT_EQ(26, fmr::math::poly_terms (3, fmr::math::Poly::Serendipity, 2) );
  EXPECT_EQ(20, fmr::math::poly_terms (3, fmr::math::Poly::Full, 3) );
}
TEST( MeshElemNodes, Pyramid ){
  EXPECT_EQ( 5, fmr::math::poly_terms (3, fmr::math::Poly::Pyramid, 1) );
  EXPECT_EQ( 5+8, fmr::math::poly_terms (3, fmr::math::Poly::Pyramid, 2) );
//TODO  EXPECT_EQ( 5+8*2, fmr::math::poly_terms (3, fmr::math::Poly::Pyramid, 3) );
}
TEST( MeshElemNodes, Prism ){
  EXPECT_EQ( 6, fmr::math::poly_terms (3, fmr::math::Poly::Prism, 1) );
  EXPECT_EQ( 6+9, fmr::math::poly_terms (3, fmr::math::Poly::Prism, 2) );
//TODO  EXPECT_EQ( 6+9*2, fmr::math::poly_terms (3, fmr::math::Poly::Prism, 3) );
}
int main(int argc, char** argv ){
  // gtest run_all_tests() is done during frm::exit(err).
  return fmr::exit (fmr::init (&argc,argv) );
}
