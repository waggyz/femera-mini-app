#ifndef FMR_HAS_MATH_QUAD_IPP
#define FMR_HAS_MATH_QUAD_IPP

//#define FMR_DEBUG

namespace fmr { namespace math {

template <std::uint_fast8_t N, std::uint_fast8_t D, typename F, typename Enable>
constexpr F gaus_lege_quad<N, D, F, Enable>::point_value(const fuint i) {// moved this mess to the end
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
template <std::uint_fast8_t N, std::uint_fast8_t D, typename F, typename Enable>
constexpr F gaus_lege_quad<N, D, F, Enable>::weight_value(
    const std::uint_fast8_t i) {
    return N == 1 ? F(2.0) :
        N == 2 ? F(1.0) :
        N == 3 ? (i == 0 ? F(8.0 / 9.0) : F(5.0 / 9.0)) :
        N == 4 ? (i == 0 || i == 3 ? F(0.34785484513745385737) :
                  F(0.65214515486254614263)) :
        (i == 0 ? F(128.0 / 225.0) :
         i == 1 || i == 2 ? F(0.47862867049936646804) :
         F(0.23692688505618908751));
  }

template <std::uint_fast8_t N, std::uint_fast8_t D, typename F, typename Enable>
gaus_jaco_quad<N, D, F, Enable>::gaus_jaco_quad(
    const F alpha, const F beta) : alpha_(alpha), beta_(beta) {
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

} }//end fmr::math namespace
#undef FMR_DEBUG
//end FMR_HAS_MATH_QUAD_IPP
#endif
