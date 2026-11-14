#include "elem-poly.hpp"

#include <gtest/gtest.h>

namespace {

template <fmr::Local_int NodeN, typename Values, typename Gradient,
          typename TransposedGradient>
void expect_bar_gradient_matches_finite_difference(
  const double (&reference_nodes)[NodeN], Values values, Gradient gradient,
  TransposedGradient transposed_gradient) {
  const double points[] = {0.17, 0.43, 0.76};
  const double h = 1.0e-6;
  for(const double xi : points){
    SCOPED_TRACE(xi);
    const double x[3] = {xi, 0.0, 0.0};
    double dN[NodeN];
    double dN_transposed[NodeN];
    gradient(dN, x);
    transposed_gradient(dN_transposed, x);
    double derivative_sum = 0.0;
    for(fmr::Local_int node=0; node<NodeN; ++node){
      const double x_plus[3] = {xi+h, 0.0, 0.0};
      const double x_minus[3] = {xi-h, 0.0, 0.0};
      double N_plus[NodeN];
      double N_minus[NodeN];
      values(N_plus, x_plus);
      values(N_minus, x_minus);
      const double numerical = (N_plus[node]-N_minus[node])/(2.0*h);
      EXPECT_NEAR(dN[node], numerical, 1.0e-8);
      EXPECT_DOUBLE_EQ(dN_transposed[node], dN[node]);
      derivative_sum += dN[node];
    }
    EXPECT_NEAR(derivative_sum, 0.0, 1.0e-12);
  }
  for(fmr::Local_int node=0; node<NodeN; ++node){
    const double x[3] = {reference_nodes[node], 0.0, 0.0};
    double N[NodeN];
    values(N, x);
    for(fmr::Local_int basis=0; basis<NodeN; ++basis){
      EXPECT_NEAR(N[basis], basis == node ? 1.0 : 0.0, 1.0e-14);
    }
  }
}

}

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

TEST(FmrElemPoly, TrivialTest) {
  EXPECT_EQ( 1, 1 );
}

TEST(FmrElemPoly, BarReferenceGradientsMatchFiniteDifferences) {
  const double p1_nodes[2] = {0.0, 1.0};
  const double p2_nodes[3] = {0.0, 1.0, 0.5};
  const double p3_nodes[4] = {0.0, 1.0, 1.0/3.0, 2.0/3.0};
  expect_bar_gradient_matches_finite_difference<2>(
    p1_nodes,
    [](double* N, const double* x) {
      return fmr::fems::elem::calc_poly_func_C1D2N(N, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_grad_node_C1D2N(dN, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_node_grad_C1D2N(dN, x);
    });
  expect_bar_gradient_matches_finite_difference<3>(
    p2_nodes,
    [](double* N, const double* x) {
      return fmr::fems::elem::calc_poly_func_C1D3N(N, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_grad_node_C1D3N(dN, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_node_grad_C1D3N(dN, x);
    });
  expect_bar_gradient_matches_finite_difference<4>(
    p3_nodes,
    [](double* N, const double* x) {
      return fmr::fems::elem::calc_poly_func_C1D4N(N, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_grad_node_C1D4N(dN, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_node_grad_C1D4N(dN, x);
    });
}
