#include "fmr.hpp"
#include "math.hpp"

#include <cmath>
#include <limits>
#include <gtest/gtest.h>

int main (int argc, char** argv) {
  return femera::test::early_main (&argc, argv);
}

namespace {

using fmr::math::Poly;

// Count monomials directly, independently of the dimension formula.
uint count_serendipity_monomials(uint variables, uint remaining_degree) {
  if (variables == 0) {return 1;}
  uint count = 2 * count_serendipity_monomials(variables - 1, remaining_degree);
  for (uint exponent = 2; exponent <= remaining_degree; ++exponent) {
    count += count_serendipity_monomials(variables - 1, remaining_degree - exponent);
  }
  return count;
}

TEST(FmrMath, DivideCeil) {
  EXPECT_EQ(fmr::math::divide_ceil(0u, 3u), 0u);
  EXPECT_EQ(fmr::math::divide_ceil(1u, 3u), 1u);
  EXPECT_EQ(fmr::math::divide_ceil(6u, 3u), 2u);
  EXPECT_EQ(fmr::math::divide_ceil(7u, 3u), 3u);
  EXPECT_EQ(fmr::math::divide_ceil(7, 1), 7);
}

TEST(FmrMath, BinaryDigits) {
  EXPECT_EQ(fmr::math::base2_digits(0), 0u);
  for (uint bit = 0; bit < std::numeric_limits<uint>::digits; ++bit) {
    const uint power = uint(1) << bit;
    EXPECT_EQ(fmr::math::base2_digits(power), bit + 1);
    EXPECT_EQ(fmr::math::base2_digits(power - 1), bit);
  }
  EXPECT_EQ(fmr::math::base2_digits(std::numeric_limits<uint>::max()),
            std::numeric_limits<uint>::digits);
}

TEST(FmrMath, DecimalDigits) {
  EXPECT_EQ(fmr::math::count_digits(0), 0u);
  uint power = 1;
  for (uint digits = 1; digits <= 10; ++digits) {
    EXPECT_EQ(fmr::math::count_digits(power), digits);
    if (digits > 1) {
      EXPECT_EQ(fmr::math::count_digits(power - 1), digits - 1);
    }
    if (digits < 10) {power *= 10;}
  }
  EXPECT_EQ(fmr::math::count_digits(std::numeric_limits<uint>::max()), 10u);
}

TEST(FmrMath, UnsignedPowers) {
  EXPECT_EQ(fmr::math::upow(0, 0), 1u);
  EXPECT_EQ(fmr::math::upow(0, 3), 0u);
  EXPECT_EQ(fmr::math::upow(1, 100), 1u);
  EXPECT_EQ(fmr::math::upow(7, 1), 7u);
  EXPECT_EQ(fmr::math::upow(2, 10), 1024u);
  EXPECT_EQ(fmr::math::upow(3, 5), 243u);
}

TEST(FmrMath, Combinations) {
  const uint pascal[][7] = {
    {1, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0},
    {1, 2, 1, 0, 0, 0, 0},
    {1, 3, 3, 1, 0, 0, 0},
    {1, 4, 6, 4, 1, 0, 0},
    {1, 5, 10, 10, 5, 1, 0},
    {1, 6, 15, 20, 15, 6, 1}
  };
  for (uint n = 0; n <= 6; ++n) {
    for (uint k = 0; k <= n; ++k) {
      EXPECT_EQ(fmr::math::nchoosek(n, k), pascal[n][k]) << n << ", " << k;
    }
  }
  EXPECT_EQ(fmr::math::nchoosek(20, 10), 184756u);
  EXPECT_EQ(fmr::math::nchoosek(65538, 2), 2147581953u);
}

TEST(FmrMathPoly, NamesCoverEnumValues) {
  for (int value = static_cast<int>(Poly::None);
       value <= static_cast<int>(Poly::end); ++value) {
    const auto found = fmr::math::poly_letter_name.find(static_cast<Poly>(value));
    ASSERT_NE(found, fmr::math::poly_letter_name.end());
    EXPECT_FALSE(found->second.first.empty());
    EXPECT_FALSE(found->second.second.empty());
  }
  EXPECT_EQ(fmr::math::poly_letter_name.at(Poly::Serendipity).first, "S");
}

TEST(FmrMathPoly, ZeroVariablesAndOrder) {
  for (int value = static_cast<int>(Poly::None);
       value <= static_cast<int>(Poly::end); ++value) {
    const Poly type = static_cast<Poly>(value);
    EXPECT_EQ(fmr::math::poly_term_n(type, 0, 0), 0u) << value;
    EXPECT_EQ(fmr::math::poly_term_n(type, 3, 0), 0u) << value;
    EXPECT_EQ(fmr::math::poly_term_n(type, 0, 3), 1u) << value;
  }
}

TEST(FmrMathPoly, FullAndTensorProducts) {
  EXPECT_EQ(fmr::math::poly_term_n(Poly::Full, 3, 1), 4u);
  EXPECT_EQ(fmr::math::poly_term_n(Poly::Full, 2, 2), 6u);
  EXPECT_EQ(fmr::math::poly_term_n(Poly::Full, 3, 3), 20u);
  EXPECT_EQ(fmr::math::poly_term_n(Poly::Full, 2, 4), 15u);
  for (Poly type : {Poly::Bipoly, Poly::Tripoly}) {
    EXPECT_EQ(fmr::math::poly_term_n(type, 3, 1), 4u);
    EXPECT_EQ(fmr::math::poly_term_n(type, 2, 2), 9u);
    EXPECT_EQ(fmr::math::poly_term_n(type, 3, 3), 64u);
    EXPECT_EQ(fmr::math::poly_term_n(type, 2, 4), 81u);
  }
}

TEST(FmrMathPoly, SerendipityPolynomialTermCount) {
  // https://homepages.math.uic.edu/~awanou/serendipity2011.pdf
  const uint expected[][6] = {
    {1, 2, 3, 4, 5, 6},
    {1, 4, 8, 12, 17, 23},
    {1, 8, 20, 32, 50, 74}
  };
  for (uint variables = 1; variables <= 3; ++variables) {
    for (uint order = 0; order <= 5; ++order) {
      EXPECT_EQ(fmr::math::poly_term_n(Poly::Serendipity, order, variables),
                expected[variables - 1][order])
          << "order " << order << ", variables " << variables;
    }
  }
  EXPECT_EQ(fmr::math::poly_term_n(Poly::Serendipity, 65539, 2), 2147778572u);
}

TEST(FmrMathPoly, SerendipityMatchesMonomialEnumeration) {
  for (uint variables = 1; variables <= 5; ++variables) {
    for (uint order = 1; order <= 8; ++order) {
      EXPECT_EQ(fmr::math::poly_term_n(Poly::Serendipity, order, variables),
                count_serendipity_monomials(variables, order))
          << "order " << order << ", variables " << variables;
    }
  }
}

TEST(FmrMathPoly, PyramidAndPrismSupportedOrders) {
  const uint pyramid[] = {5, 13, 21};
  const uint prism[] = {6, 15, 24};
  for (uint order = 1; order <= 3; ++order) {
    EXPECT_EQ(fmr::math::poly_term_n(Poly::Pyramid, order, 3), pyramid[order - 1]);
    EXPECT_EQ(fmr::math::poly_term_n(Poly::Prism, order, 3), prism[order - 1]);
  }
  EXPECT_EQ(fmr::math::poly_term_n(Poly::Pyramid, 4, 3), 0u);
  EXPECT_EQ(fmr::math::poly_term_n(Poly::Prism, 4, 3), 0u);
}

TEST(FmrMathPoly, NonPolynomialTypes) {
  for (Poly type : {Poly::None, Poly::Error, Poly::Unknown, Poly::end}) {
    EXPECT_EQ(fmr::math::poly_term_n(type, 2, 3), 0u);
  }
  EXPECT_EQ(fmr::math::poly_term_n(static_cast<Poly>(100), 2, 3), 0u);
}

template <typename F>
void check_floating_comparisons() {
  const F one = F(1);
  const F adjacent = std::nextafter(one, F(2));
  const F separated = one + F(4) * std::numeric_limits<F>::epsilon();
  EXPECT_TRUE(fmr::math::are_close(one, adjacent));
  EXPECT_TRUE(fmr::math::are_close(adjacent, one));
  EXPECT_TRUE(fmr::math::are_equal(one, adjacent));
  EXPECT_FALSE(fmr::math::are_close(one, separated));
  EXPECT_FALSE(fmr::math::are_equal(one, separated));
  EXPECT_TRUE(fmr::math::are_equal(F(0), F(-0.0)));
  EXPECT_FALSE(fmr::math::are_close(F(0), std::numeric_limits<F>::min()));
  EXPECT_FALSE(fmr::math::is_less_than(one, adjacent));
  EXPECT_FALSE(fmr::math::is_more_than(adjacent, one));
  EXPECT_FALSE(fmr::math::is_greater_than(adjacent, one));
  EXPECT_TRUE(fmr::math::is_less_than(one, separated));
  EXPECT_TRUE(fmr::math::is_more_than(separated, one));
  EXPECT_TRUE(fmr::math::is_greater_than(separated, one));
  EXPECT_TRUE(fmr::math::is_less_than(F(-2), F(-1)));
  EXPECT_FALSE(fmr::math::is_less_than(separated, one));
  EXPECT_FALSE(fmr::math::is_more_than(one, separated));
  EXPECT_FALSE(fmr::math::is_greater_than(one, separated));
  const F nan = std::numeric_limits<F>::quiet_NaN();
  EXPECT_FALSE(fmr::math::are_close(nan, one));
  EXPECT_FALSE(fmr::math::are_equal(nan, one));
  EXPECT_FALSE(fmr::math::is_less_than(nan, one));
  EXPECT_FALSE(fmr::math::is_more_than(nan, one));
  EXPECT_FALSE(fmr::math::is_greater_than(nan, one));
}

TEST(FmrMath, FloatingPointComparisons) {
  check_floating_comparisons<float>();
  check_floating_comparisons<double>();
  check_floating_comparisons<long double>();
}

} // namespace
