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
  static constexpr F point_value(const fuint i);
  static constexpr F weight_value(const fuint i);
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
};//end struct gaus_lege_quad

template <std::uint_fast8_t N, std::uint_fast8_t D=1, typename F=double,
typename = std::enable_if_t<std::is_floating_point<F>::value>>
/** Gauss-Jacobi rule for integrating f(x)/sqrt(x) on [0, 1]. */
struct sqrt_sing_quad {
  private:
  using fuint = std::uint_fast8_t;
  gaus_jaco_quad<N, D, F> rule_{F(0), F(-0.5)};

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
  F integrate(F* values) const { return integrate(static_cast<const F*>(values)); }
};//end struct sqrt_sing_quad

template <std::uint_fast8_t N, std::uint_fast8_t D=1, typename F=double,
typename = std::enable_if_t<std::is_floating_point<F>::value>>
/** N-point Gauss-Jacobi quadrature for (1-x)^alpha (1+x)^beta on [-1, 1]. */
struct gaus_jaco_quad {
  private:
  using fuint = std::uint_fast8_t;
  std::array<F, N> points_{};
  std::array<F, N> weights_{};
  F alpha_;
  F beta_;

  public:
  static_assert(N >= 1, "gaus_jaco_quad requires at least one point");
  static constexpr fuint get_num_points() { return N; }
  static constexpr F get_interval_start() { return F(-1); }
  static constexpr F get_interval_end() { return F(1); }

  gaus_jaco_quad(const F alpha, const F beta) : alpha_(alpha), beta_(beta) {
    if (!(alpha > F(-1)) || !(beta > F(-1)) ||
        !std::isfinite(alpha) || !std::isfinite(beta)) {
      throw std::invalid_argument("gaus_jaco_quad requires alpha and beta > -1");
    }

    using matrix = std::array<std::array<F, N>, N>;
    matrix jacobi{};
    matrix eigenvectors{};
    const F sum = alpha + beta;

    for (fuint i = 0; i < N; ++i) {
      eigenvectors[i][i] = F(1);
      const F k = static_cast<F>(i);
      jacobi[i][i] = i == 0 ? (beta - alpha) / (sum + F(2)) :
          (beta * beta - alpha * alpha) /
          ((F(2) * k + sum) * (F(2) * k + sum + F(2)));
      if (i > 0) {
        const F factor = i == 1 ? F(1) :
            (k + sum) / (F(2) * k + sum - F(1));
        const F off_diagonal = F(2) * std::sqrt(
            k * (k + alpha) * (k + beta) * factor /
            ((F(2) * k + sum) * (F(2) * k + sum) *
             (F(2) * k + sum + F(1))));
        jacobi[i - 1][i] = off_diagonal;
        jacobi[i][i - 1] = off_diagonal;
      }
    }

    const F tolerance = std::numeric_limits<F>::epsilon() * F(8);
    const std::size_t max_iterations = std::size_t(50) * N * N;
    bool converged = false;
    for (std::size_t iteration = 0; iteration < max_iterations; ++iteration) {
      fuint p = 0;
      fuint q = 0;
      F largest = F(0);
      F scale = F(1);
      for (fuint i = 0; i < N; ++i) {
        scale = std::max(scale, std::abs(jacobi[i][i]));
        for (fuint j = static_cast<fuint>(i + 1); j < N; ++j) {
          const F magnitude = std::abs(jacobi[i][j]);
          if (magnitude > largest) {
            largest = magnitude;
            p = i;
            q = j;
          }
        }
      }
      if (largest <= tolerance * scale) {
        converged = true;
        break;
      }

      const F apq = jacobi[p][q];
      const F tau = (jacobi[q][q] - jacobi[p][p]) / (F(2) * apq);
      const F t = (tau >= F(0) ? F(1) : F(-1)) /
          (std::abs(tau) + std::sqrt(F(1) + tau * tau));
      const F c = F(1) / std::sqrt(F(1) + t * t);
      const F s = t * c;
      const F app = jacobi[p][p];
      const F aqq = jacobi[q][q];
      jacobi[p][p] = app - t * apq;
      jacobi[q][q] = aqq + t * apq;
      jacobi[p][q] = F(0);
      jacobi[q][p] = F(0);

      for (fuint k = 0; k < N; ++k) {
        if (k != p && k != q) {
          const F akp = jacobi[k][p];
          const F akq = jacobi[k][q];
          jacobi[k][p] = c * akp - s * akq;
          jacobi[p][k] = jacobi[k][p];
          jacobi[k][q] = s * akp + c * akq;
          jacobi[q][k] = jacobi[k][q];
        }
        const F vkp = eigenvectors[k][p];
        const F vkq = eigenvectors[k][q];
        eigenvectors[k][p] = c * vkp - s * vkq;
        eigenvectors[k][q] = s * vkp + c * vkq;
      }
    }
    if (!converged) {
      throw std::runtime_error("gaus_jaco_quad eigensolver did not converge");
    }

    const F moment_zero = std::pow(F(2), sum + F(1)) *
        std::tgamma(alpha + F(1)) * std::tgamma(beta + F(1)) /
        std::tgamma(sum + F(2));
    if (!std::isfinite(moment_zero) || !(moment_zero > F(0))) {
      throw std::overflow_error("gaus_jaco_quad weight normalization is invalid");
    }
    for (fuint i = 0; i < N; ++i) {
      points_[i] = jacobi[i][i];
      weights_[i] = moment_zero * eigenvectors[0][i] * eigenvectors[0][i];
    }
    for (fuint i = 0; i < N; ++i) {
      for (fuint j = static_cast<fuint>(i + 1); j < N; ++j) {
        if (points_[j] < points_[i]) {
          std::swap(points_[i], points_[j]);
          std::swap(weights_[i], weights_[j]);
        }
      }
    }
  }

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
  F integrate(F* values) const { return integrate(static_cast<const F*>(values)); }
};//end struct gaus_jaco_quad

} }//end fmr::math namespace

#include "quad.ipp"

#undef FMR_DEBUG
#endif // FMR_QUAD_HPP