#include "fmr.hpp"
#include "math.hpp"

#include <gtest/gtest.h>

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

TEST( FmrMath, DidRunTest) {
  EXPECT_TRUE(true);
}

namespace fmr {
TEST(FmrMathPoly, SerendipityPolynomialTermCount) {
  //fmr::math::poly_term_n (Poly ptype, uint pord, uint nvars)
  //cite: https://homepages.math.uic.edu/~awanou/serendipity2011.pdf
  // Bars
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 1, 1), 2 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 2, 1), 3 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 3, 1), 4 );
  // Quad
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 1, 2), 4 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 2, 2), 8 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 3, 2),12 );
  // Cube
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 1, 3), 8 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 2, 3),20 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 3, 3),32 );
}
//
}//end namespace fmr::