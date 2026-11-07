#ifndef FMR_QUAD_HPP
#define FMR_QUAD_HPP

#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

//#include <immintrin.h>// for optimzaiton later, if needed

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <iostream>
#endif

namespace fmr { namespace math {

template <std::uint_fast8_t N, std::uint_fast8_t D=1, typename F=double,
typename = std::enable_if_t<std::is_floating_point<F>::value>>
/** N: N-point rule, D: dimension (default 1), F: floating-point type */
struct gaus_lege_quad{
  /* N-point Gauss-Legendre quadrature rule for an interval [a, b]
  * Typical use:
  *   1. Implement a rule struct.
  *   2. Retrieve quadrature points and weights.
  *   3. Fill (thread-)local arrays with function values at the quadrature points.
  *   4. Compute the integral.
  *
  * Example usage to integrate float x*x*x + 1 over [0, 1] using 2-point rule:
  *   //
  *   // Set up the quadrature rule.
  *   constexpr std::uint_fast8_t rule_n = 2;
  *   constexpr auto rule = gaus_lege_quad<rule_n,1,float>();
  *   //
  *   // Create and initialize (thread-)local arrays.
  *   float pts[rule_n]; rule.get_points(pts, 0.0f, 1.0f);
  *   float vals[rule_n];// Does not need initialization for this example.
  *   //
  *   // Evaluate the function at the quadrature points.
  *   for (std::uint_fast8_t i = 0; i < rule_n; ++i) {
  *     vals[i] = pts[i] * pts[i] * pts[i] + 1.0f;
  *   }
  *   // Integrate.
  *   float result = rule.integrate(vals, 0.0f, 1.0f);
  * 
  * ref: https://en.wikipedia.org/wiki/Gauss%E2%80%93Legendre_quadrature
  */
  private:
  using fuint = std::uint_fast8_t;
  static_assert(N >= 1 && N <= 5,
      "gaus_lege_quad currently supports 1- through 5-point rules");
  static constexpr F interval[2] = {F(-1.0), F(1.0)};
  public:
  static constexpr fuint rule_for_degree( const fuint degree) {
    return degree <= 9 ? static_cast<fuint>((degree + 2) / 2) :
      throw std::out_of_range(
        "gaus_lege_quad supports polynomial degrees 0 through 9");
  }
  static constexpr fuint polynomial_degree_max() { return 2 * N - 1; }
  static constexpr fuint get_num_points() { return N; }
  static constexpr F get_interval_start() { return interval[0]; }
  static constexpr F get_interval_end() { return interval[1]; }
  static constexpr F get_point(const fuint i) { return point_value(i); }
  static constexpr F get_weight(const fuint i) { return weight_value(i); }
  static constexpr
  F jacobian (const F a, const F b) { return F(0.5) * F(b - a); }
  // Array methods
  static constexpr
  void get_weights(F* w) {for (fuint i = 0; i < N; ++i) w[i] = weight_value(i);}
  static constexpr
  void get_points(F* p) { for (fuint i = 0; i < N; ++i) p[i] = point_value(i); }
  static constexpr
  void get_points(F* p, const F a, const F b) {
    for (fuint i = 0; i < N; ++i) {
      p[i] = F(0.5) * (F(b) - F(a)) * point_value(i) + F(0.5) * (F(a) + F(b));
    }
  }
  static constexpr
  F integrate (F* f_values) {
    F integral = F(0.0);
    for (fuint i = 0; i < N; ++i) { integral += weight_value(i) * f_values[i]; }
    return integral;
  }
  static constexpr
  F integrate (F* f_values, const F a, const F b) {
    F integral = F(0.0);
    const F jac = jacobian(a, b);
    for (fuint i = 0; i < N; ++i) { integral += weight_value(i) * f_values[i]; }
    return integral * jac;
  }
  private:
  static constexpr F point_value(const fuint i);
  static constexpr F weight_value(const fuint i);
};//end struct gaus_lege_quad

template <std::uint_fast8_t N, std::uint_fast8_t D=1, typename F=double,
typename = std::enable_if_t<std::is_floating_point<F>::value>>
/** N-point Gauss-Jacobi quadrature for (1-x)^alpha (1+x)^beta on [-1, 1]. */
struct gaus_jaco_quad {
  private:
  using fuint = std::uint_fast8_t;
  public:
  static_assert(N >= 1, "gaus_jaco_quad requires at least one point");
  gaus_jaco_quad(const F alpha, const F beta);// constructor declaration
  static constexpr fuint get_num_points() { return N; }
  static constexpr F get_interval_start() { return F(-1); }
  static constexpr F get_interval_end() { return F(1); }
  //
  F get_alpha() const { return alpha_; }
  F get_beta() const { return beta_; }
  F get_point(const fuint i) const { return points_[i]; }
  F get_weight(const fuint i) const { return weights_[i]; }
  void get_points(F* points) const {
    for (fuint i = 0; i < N; ++i) { points[i] = points_[i]; }
  }
  void get_weights(F* weights) const {
    for (fuint i = 0; i < N; ++i) { weights[i] = weights_[i]; }
  }
  F integrate(const F* values) const {
    F result = F(0);
    for (fuint i = 0; i < N; ++i) { result += weights_[i] * values[i]; }
    return result;
  }
  F integrate(F* values) const {return integrate(static_cast<const F*>(values));}
  private:
  std::array<F, N> points_{};
  std::array<F, N> weights_{};
  F alpha_;
  F beta_;
};//end struct gaus_jaco_quad

template <std::uint_fast8_t N, std::uint_fast8_t D=1, typename F=double,
typename = std::enable_if_t<std::is_floating_point<F>::value>>
/** Gauss-Jacobi rule for integrating f(x)/sqrt(x) on [0, 1]. */
struct sqrt_sing_quad {
  private:
  using fuint = std::uint_fast8_t;
  public:
  static constexpr fuint get_num_points() { return N; }
  static constexpr F get_interval_start() { return F(0); }
  static constexpr F get_interval_end() { return F(1); }
  F get_point(const fuint i) const {
    return F(0.5) * (rule_.get_point(i) + F(1));
  }
  F get_weight(const fuint i) const {
    return rule_.get_weight(i) / std::sqrt(F(2));
  }
  void get_points(F* points) const {
    for (fuint i = 0; i < N; ++i) { points[i] = get_point(i); }
  }
  void get_weights(F* weights) const {
    for (fuint i = 0; i < N; ++i) { weights[i] = get_weight(i); }
  }
  F integrate(const F* values) const {
    F result = F(0);
    for (fuint i = 0; i < N; ++i) { result += get_weight(i) * values[i]; }
    return result;
  }
  F integrate(F* values) const {return integrate(static_cast<const F*>(values));}
  private:
  gaus_jaco_quad<N, D, F> rule_{F(0), F(-0.5)};// magic :)
};//end struct sqrt_sing_quad

} }//end fmr::math namespace

#include "quad.ipp"

#undef FMR_DEBUG
#endif // FMR_QUAD_HPP