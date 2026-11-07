#include "fmr.hpp"
#include "quad.hpp"

#include <cmath>
#include <gtest/gtest.h>

TEST(FmrQuad, OnePointRule) {
  using Rule = fmr::math::gaus_lege_quad<1>;

  EXPECT_EQ(Rule::get_num_points(), 1);
  EXPECT_DOUBLE_EQ(Rule::get_interval_start(), -1.0);
  EXPECT_DOUBLE_EQ(Rule::get_interval_end(), 1.0);
  EXPECT_DOUBLE_EQ(Rule::get_point(0), 0.0);
  EXPECT_DOUBLE_EQ(Rule::get_weight(0), 2.0);

  double points[1];
  double weights[1];
  Rule::get_points(points);
  Rule::get_weights(weights);
  EXPECT_DOUBLE_EQ(points[0], 0.0);
  EXPECT_DOUBLE_EQ(weights[0], 2.0);
  EXPECT_DOUBLE_EQ(Rule::integrate(points), 0.0);
}

TEST(FmrQuad, TwoPointRule) {
  using Rule = fmr::math::gaus_lege_quad<2>;
  const double node = 1.0 / std::sqrt(3.0);

  EXPECT_EQ(Rule::get_num_points(), 2);
  EXPECT_NEAR(Rule::get_point(0), node, 1e-15);
  EXPECT_NEAR(Rule::get_point(1), -node, 1e-15);
  EXPECT_DOUBLE_EQ(Rule::get_weight(0), 1.0);
  EXPECT_DOUBLE_EQ(Rule::get_weight(1), 1.0);

  double points[2];
  double weights[2];
  Rule::get_points(points);
  Rule::get_weights(weights);
  EXPECT_NEAR(points[0], node, 1e-15);
  EXPECT_NEAR(points[1], -node, 1e-15);
  EXPECT_DOUBLE_EQ(weights[0], 1.0);
  EXPECT_DOUBLE_EQ(weights[1], 1.0);

  double values[] = {points[0] * points[0], points[1] * points[1]};
  EXPECT_NEAR(Rule::integrate(values), 2.0 / 3.0, 1e-15);
}

TEST(FmrQuad, ThreePointRule) {
  using Rule = fmr::math::gaus_lege_quad<3>;
  const double node = std::sqrt(3.0 / 5.0);

  EXPECT_EQ(Rule::get_num_points(), 3);
  EXPECT_DOUBLE_EQ(Rule::get_point(0), 0.0);
  EXPECT_NEAR(Rule::get_point(1), node, 1e-15);
  EXPECT_NEAR(Rule::get_point(2), -node, 1e-15);
  EXPECT_NEAR(Rule::get_weight(0), 8.0 / 9.0, 1e-15);
  EXPECT_NEAR(Rule::get_weight(1), 5.0 / 9.0, 1e-15);
  EXPECT_NEAR(Rule::get_weight(2), 5.0 / 9.0, 1e-15);

  double points[3];
  double weights[3];
  Rule::get_points(points);
  Rule::get_weights(weights);
  for (std::uint_fast8_t i = 0; i < 3; ++i) {
    EXPECT_DOUBLE_EQ(points[i], Rule::get_point(i));
    EXPECT_DOUBLE_EQ(weights[i], Rule::get_weight(i));
  }

  double values[] = {
    points[0] * points[0] * points[0] * points[0],
    points[1] * points[1] * points[1] * points[1],
    points[2] * points[2] * points[2] * points[2]
  };
  EXPECT_NEAR(Rule::integrate(values), 2.0 / 5.0, 1e-15);
}

TEST(FmrQuad, FourPointRule) {
  using Rule = fmr::math::gaus_lege_quad<4>;
  const double outer_node = 0.86113631159405257522;
  const double inner_node = 0.33998104358485626480;
  const double outer_weight = 0.34785484513745385737;
  const double inner_weight = 0.65214515486254614263;

  EXPECT_EQ(Rule::get_num_points(), 4);
  EXPECT_NEAR(Rule::get_point(0), outer_node, 1e-15);
  EXPECT_NEAR(Rule::get_point(1), inner_node, 1e-15);
  EXPECT_NEAR(Rule::get_point(2), -inner_node, 1e-15);
  EXPECT_NEAR(Rule::get_point(3), -outer_node, 1e-15);
  EXPECT_NEAR(Rule::get_weight(0), outer_weight, 1e-15);
  EXPECT_NEAR(Rule::get_weight(1), inner_weight, 1e-15);
  EXPECT_NEAR(Rule::get_weight(2), inner_weight, 1e-15);
  EXPECT_NEAR(Rule::get_weight(3), outer_weight, 1e-15);

  double points[4];
  double weights[4];
  Rule::get_points(points);
  Rule::get_weights(weights);
  for (std::uint_fast8_t i = 0; i < 4; ++i) {
    EXPECT_DOUBLE_EQ(points[i], Rule::get_point(i));
    EXPECT_DOUBLE_EQ(weights[i], Rule::get_weight(i));
  }

  double values[4];
  for (std::uint_fast8_t i = 0; i < 4; ++i) {
    values[i] = points[i] * points[i] * points[i] * points[i] *
        points[i] * points[i];
  }
  EXPECT_NEAR(Rule::integrate(values), 2.0 / 7.0, 1e-15);
}

TEST(FmrQuad, FivePointRule) {
  using Rule = fmr::math::gaus_lege_quad<5>;
  const double inner_node = 0.53846931010568309104;
  const double outer_node = 0.90617984593866399280;
  const double center_weight = 128.0 / 225.0;
  const double inner_weight = 0.47862867049936646804;
  const double outer_weight = 0.23692688505618908751;

  EXPECT_EQ(Rule::get_num_points(), 5);
  EXPECT_DOUBLE_EQ(Rule::get_point(0), 0.0);
  EXPECT_NEAR(Rule::get_point(1), inner_node, 1e-15);
  EXPECT_NEAR(Rule::get_point(2), -inner_node, 1e-15);
  EXPECT_NEAR(Rule::get_point(3), outer_node, 1e-15);
  EXPECT_NEAR(Rule::get_point(4), -outer_node, 1e-15);
  EXPECT_NEAR(Rule::get_weight(0), center_weight, 1e-15);
  EXPECT_NEAR(Rule::get_weight(1), inner_weight, 1e-15);
  EXPECT_NEAR(Rule::get_weight(2), inner_weight, 1e-15);
  EXPECT_NEAR(Rule::get_weight(3), outer_weight, 1e-15);
  EXPECT_NEAR(Rule::get_weight(4), outer_weight, 1e-15);

  double points[5];
  double weights[5];
  Rule::get_points(points);
  Rule::get_weights(weights);
  for (std::uint_fast8_t i = 0; i < 5; ++i) {
    EXPECT_DOUBLE_EQ(points[i], Rule::get_point(i));
    EXPECT_DOUBLE_EQ(weights[i], Rule::get_weight(i));
  }

  double values[5];
  for (std::uint_fast8_t i = 0; i < 5; ++i) {
    values[i] = points[i] * points[i] * points[i] * points[i] *
        points[i] * points[i] * points[i] * points[i];
  }
  EXPECT_NEAR(Rule::integrate(values), 2.0 / 9.0, 1e-15);
}

TEST(FmrQuad, MapsPointsAndScalesIntegralToInterval) {
  using Rule = fmr::math::gaus_lege_quad<2>;
  const double a = 2.0;
  const double b = 6.0;
  const double node = 1.0 / std::sqrt(3.0);
  double points[2];

  Rule::get_points(points, a, b);
  EXPECT_NEAR(points[0], 4.0 + 2.0 * node, 1e-15);
  EXPECT_NEAR(points[1], 4.0 - 2.0 * node, 1e-15);
  EXPECT_DOUBLE_EQ(Rule::jacobian(a, b), 2.0);

  double values[2] = {points[0] * points[0], points[1] * points[1]};
  EXPECT_NEAR(Rule::integrate(values, a, b), 208.0 / 3.0, 1e-13);
  EXPECT_DOUBLE_EQ(Rule::jacobian(b, a), -2.0);
}

TEST(FmrQuad, SupportsFloatRules) {
  using Rule = fmr::math::gaus_lege_quad<1, 1, float>;

  EXPECT_FLOAT_EQ(Rule::get_point(0), 0.0f);
  EXPECT_FLOAT_EQ(Rule::get_weight(0), 2.0f);
}

TEST(FmrQuad, MaximumPolynomialDegree) {
  using Rule1 = fmr::math::gaus_lege_quad<1>;
  using Rule2 = fmr::math::gaus_lege_quad<2>;
  using Rule3 = fmr::math::gaus_lege_quad<3>;
  using Rule4 = fmr::math::gaus_lege_quad<4>;
  using Rule5 = fmr::math::gaus_lege_quad<5>;

  static_assert(Rule1::polynomial_degree_max() == 1, "1-point rule degree");
  static_assert(Rule2::polynomial_degree_max() == 3, "2-point rule degree");
  static_assert(Rule3::polynomial_degree_max() == 5, "3-point rule degree");
  static_assert(Rule4::polynomial_degree_max() == 7, "4-point rule degree");
  static_assert(Rule5::polynomial_degree_max() == 9, "5-point rule degree");

  EXPECT_EQ(Rule1::polynomial_degree_max(), 1);
  EXPECT_EQ(Rule2::polynomial_degree_max(), 3);
  EXPECT_EQ(Rule3::polynomial_degree_max(), 5);
  EXPECT_EQ(Rule4::polynomial_degree_max(), 7);
  EXPECT_EQ(Rule5::polynomial_degree_max(), 9);
}

TEST(FmrQuad, SelectsRuleForPolynomialDegree) {
  using Rule = fmr::math::gaus_lege_quad<1>;

  static_assert(Rule::rule_for_degree(0) == 1, "constant needs one point");
  static_assert(Rule::rule_for_degree(1) == 1, "linear needs one point");
  static_assert(Rule::rule_for_degree(2) == 2, "quadratic needs two points");
  static_assert(Rule::rule_for_degree(3) == 2, "cubic needs two points");
  static_assert(Rule::rule_for_degree(4) == 3, "quartic needs three points");
  static_assert(Rule::rule_for_degree(5) == 3, "quintic needs three points");
  static_assert(Rule::rule_for_degree(6) == 4, "degree six needs four points");
  static_assert(Rule::rule_for_degree(7) == 4, "degree seven needs four points");
  static_assert(Rule::rule_for_degree(8) == 5, "degree eight needs five points");
  static_assert(Rule::rule_for_degree(9) == 5, "degree nine needs five points");

  EXPECT_EQ(Rule::rule_for_degree(0), 1);
  EXPECT_EQ(Rule::rule_for_degree(1), 1);
  EXPECT_EQ(Rule::rule_for_degree(2), 2);
  EXPECT_EQ(Rule::rule_for_degree(3), 2);
  EXPECT_EQ(Rule::rule_for_degree(4), 3);
  EXPECT_EQ(Rule::rule_for_degree(5), 3);
  EXPECT_EQ(Rule::rule_for_degree(6), 4);
  EXPECT_EQ(Rule::rule_for_degree(7), 4);
  EXPECT_EQ(Rule::rule_for_degree(8), 5);
  EXPECT_EQ(Rule::rule_for_degree(9), 5);
  EXPECT_THROW(Rule::rule_for_degree(10), std::out_of_range);
}

TEST(FmrQuad, GaussJacobiIntegratesWeightedPolynomials) {
  using Rule = fmr::math::gaus_jaco_quad<3>;
  const Rule rule(1.0, 0.0);
  double values[3];
  double points[3];
  rule.get_points(points);

  for (std::uint_fast8_t degree = 0; degree <= 5; ++degree) {
    for (std::uint_fast8_t i = 0; i < 3; ++i) {
      values[i] = std::pow(points[i], degree);
    }
    const double exact = degree % 2 == 0 ? 2.0 / (degree + 1) :
        -2.0 / (degree + 2);
    EXPECT_NEAR(rule.integrate(values), exact, 2e-14);
  }
}

TEST(FmrQuad, GaussJacobiSupportsLegendreWeightAndValidatesParameters) {
  using Rule = fmr::math::gaus_jaco_quad<3>;
  const Rule rule(0.0, 0.0);
  const double node = std::sqrt(3.0 / 5.0);

  EXPECT_NEAR(rule.get_point(0), -node, 2e-15);
  EXPECT_NEAR(rule.get_point(1), 0.0, 2e-15);
  EXPECT_NEAR(rule.get_point(2), node, 2e-15);
  EXPECT_NEAR(rule.get_weight(0), 5.0 / 9.0, 2e-15);
  EXPECT_NEAR(rule.get_weight(1), 8.0 / 9.0, 2e-15);
  EXPECT_NEAR(rule.get_weight(2), 5.0 / 9.0, 2e-15);
  EXPECT_THROW(Rule(-1.0, 0.0), std::invalid_argument);
  EXPECT_THROW(Rule(0.0, -1.0), std::invalid_argument);

  const Rule endpoint_singular(-0.5, -0.5);
  double ones[3] = {1.0, 1.0, 1.0};
  EXPECT_NEAR(endpoint_singular.integrate(ones), std::acos(-1.0), 2e-14);
}

TEST(FmrQuad, GaussJacobiIntegratesInverseSquareRootOnUnitInterval) {
  using Rule = fmr::math::sqrt_sing_quad<3>;
  const Rule rule;
  double points[3];
  double values[3];
  rule.get_points(points);

  EXPECT_DOUBLE_EQ(Rule::get_interval_start(), 0.0);
  EXPECT_DOUBLE_EQ(Rule::get_interval_end(), 1.0);
  for (std::uint_fast8_t degree = 0; degree <= 5; ++degree) {
    for (std::uint_fast8_t i = 0; i < 3; ++i) {
      values[i] = std::pow(points[i], degree);
    }
    EXPECT_NEAR(rule.integrate(values), 1.0 / (degree + 0.5), 2e-14);
  }
}

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
