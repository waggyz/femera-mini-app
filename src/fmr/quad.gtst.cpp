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

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
