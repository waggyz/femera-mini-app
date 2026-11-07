#ifndef FMR_QUAD_HPP
#define FMR_QUAD_HPP

#include <cstdint>
#include <type_traits>

//#include <immintrin.h>// for optimzaiton later, if needed

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <iostream>
#endif

namespace fmr { namespace math {

template <std::uint_fast8_t N, typename std::uint_fast8_t D=1,
typename F=double, typename = std::enable_if_t<std::is_floating_point<F>::value>>
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
  static_assert(N >= 1 && N <= 5,
      "gaus_lege_quad currently supports 1- through 5-point rules");
  static constexpr F point_value(const std::uint_fast8_t i) {
    return N == 1 ? F(0.0) :
        N == 2 ? (i == 0 ? F(0.57735026918962576451) : F(-0.57735026918962576451)) :
        N == 3 ? (i == 0 ? F(0.0) :
                  i == 1 ? F(0.77459666924148337704) : F(-0.77459666924148337704)) :
        N == 4 ? (i == 0 ? F(0.86113631159405257522) :
                  i == 1 ? F(0.33998104358485626480) :
                  i == 2 ? F(-0.33998104358485626480) : F(-0.86113631159405257522)) :
        (i == 0 ? F(0.0) :
         i == 1 ? F(0.53846931010568309104) :
         i == 2 ? F(-0.53846931010568309104) :
         i == 3 ? F(0.90617984593866399280) : F(-0.90617984593866399280));
  }
  static constexpr F weight_value(const std::uint_fast8_t i) {
    return N == 1 ? F(2.0) :
        N == 2 ? F(1.0) :
        N == 3 ? (i == 0 ? F(8.0 / 9.0) : F(5.0 / 9.0)) :
        N == 4 ? (i == 0 || i == 3 ? F(0.34785484513745385737) :
                  F(0.65214515486254614263)) :
        (i == 0 ? F(128.0 / 225.0) :
         i == 1 || i == 2 ? F(0.47862867049936646804) :
         F(0.23692688505618908751));
  }
  static constexpr F interval[2] = {F(-1.0), F(1.0)};
  public:
  static constexpr std::uint_fast8_t get_num_points() { return N; }
  static constexpr std::uint_fast8_t polynomial_degree_max() { return 2 * N - 1; }
  static constexpr F get_interval_start() { return interval[0]; }
  static constexpr F get_interval_end() { return interval[1]; }
  static constexpr F get_point(const std::uint_fast8_t i) { return point_value(i); }
  static constexpr F get_weight(const std::uint_fast8_t i) { return weight_value(i); }
  //
  static constexpr void get_weights(F* w) {
     for (std::uint_fast8_t i = 0; i < N; ++i) w[i] = weight_value(i);
    }
  static constexpr void get_points(F* p) {
     for (std::uint_fast8_t i = 0; i < N; ++i) p[i] = point_value(i);
    }
  static constexpr void get_points(F* p, const F a, const F b) {
    for (std::uint_fast8_t i = 0; i < N; ++i) {
      p[i] = F(0.5) * (F(b) - F(a)) * point_value(i) + F(0.5) * (F(a) + F(b));
    }
  }
  static constexpr F jacobian (const F a, const F b) { return F(0.5) * F(b - a); }
  static constexpr F integrate (F* f_values) {
    F integral = F(0.0);
    for (std::uint_fast8_t i = 0; i < N; ++i) {
      integral += weight_value(i) * f_values[i];
    }
    return integral;
  }
  static constexpr F integrate (F* f_values, const F a, const F b) {
    F integral = F(0.0);
    const F jac = jacobian(a, b);
    for (std::uint_fast8_t i = 0; i < N; ++i) {
      integral += weight_value(i) * f_values[i];
    }
    return integral * jac;
  }
};//end struct gaus_lege_quad

} }//end fmr::math namespace


#endif // FMR_QUAD_HPP