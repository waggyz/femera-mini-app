#include "elem-poly.hpp"
#include "../../femera/grid/elem/Tets.hpp"
#include "../math.hpp"

#include <gtest/gtest.h>
#include <cmath>

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

TEST(FmrElemPoly, TrivialTest) {
  EXPECT_EQ( 1, 1 );
}

namespace fmr {
TEST(FmrElemPoly, SerendipityPolynomialTermCount) {
  // use to check node count
  //fmr::math::poly_term_n (Poly ptype, uint pord, uint nvars)
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

namespace {

template <typename T>
constexpr bool constexpr_near(T actual, T expected, T tolerance) {
  const T difference = actual - expected;
  return difference >= -tolerance && difference <= tolerance;
}

constexpr bool triangle_constexpr_test() {
  const double x[3] = {0.0, 0.0, 0.0};
  double f[10] = {};
  double g[20] = {};
  double gt[20] = {};
  return fmr::fems::elem::calc_poly_func_T2D10N(f, x) == f
    && fmr::fems::elem::calc_poly_grad_node_T2D10N(g, x) == g
    && fmr::fems::elem::calc_poly_node_grad_T2D10N(gt, x) == gt
    && constexpr_near(f[0], 1.0, 1.0e-14)
    && constexpr_near(f[9], 0.0, 1.0e-14)
    && constexpr_near(g[0], -5.5, 1.0e-14)
    && constexpr_near(gt[0], g[0], 1.0e-14);
}
static_assert(triangle_constexpr_test(), "cubic triangle must be constexpr");
constexpr bool triangle_mixed_precision_test() {
  const double x[3] = {0.0, 0.0, 0.0};
  float f[10] = {};
  float g[20] = {};
  return fmr::fems::elem::calc_poly_func_T2D10N(f, x) == f
    && fmr::fems::elem::calc_poly_grad_node_T2D10N(g, x) == g
    && constexpr_near(f[0], 1.0f, 1.0e-6f)
    && constexpr_near(g[0], -5.5f, 1.0e-6f);
}
static_assert(triangle_mixed_precision_test(), "triangle supports mixed precision");

constexpr bool incomplete_triangle_constexpr_test() {
  const double x[3] = {0.0, 0.0, 0.0};
  float f[9] = {}, g[18] = {}, gt[18] = {};
  return fmr::fems::elem::calc_poly_func_T2D9N(f, x) == f
    && fmr::fems::elem::calc_poly_grad_node_T2D9N(g, x) == g
    && fmr::fems::elem::calc_poly_node_grad_T2D9N(gt, x) == gt
    && constexpr_near(f[0], 1.0f, 1.0e-6f)
    && constexpr_near(g[0], -5.5f, 1.0e-6f)
    && constexpr_near(gt[0], g[0], 1.0e-6f);
}
static_assert(incomplete_triangle_constexpr_test(), "Tri9 must be constexpr");

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

}// end anonymous namespace

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

TEST(FmrElemPoly, CubicTriangleBasisAndGradients) {
  const double nodes[10][3] = {
    {0,0,0}, {1,0,0}, {0,1,0},
    {1.0/3,0,0}, {2.0/3,0,0},
    {2.0/3,1.0/3,0}, {1.0/3,2.0/3,0},
    {0,2.0/3,0}, {0,1.0/3,0},
    {1.0/3,1.0/3,0}
  };
  for (int n=0; n<10; ++n) {
    double f[10] = {};
    fmr::fems::elem::calc_poly_func_T2D10N(f, nodes[n]);
    for (int i=0; i<10; ++i) EXPECT_NEAR(f[i], i==n ? 1.0 : 0.0, 1e-14);
  }
  const double points[3][3] = {{0.17,0.21,0}, {0.43,0.13,0}, {0.0,0.5,0}};
  for (const auto& x : points) {
    double f[10] = {}, g[20] = {}, gt[20] = {};
    fmr::fems::elem::calc_poly_func_T2D10N(f, x);
    fmr::fems::elem::calc_poly_grad_node_T2D10N(g, x);
    fmr::fems::elem::calc_poly_node_grad_T2D10N(gt, x);
    for (int d=0; d<2; ++d) {
      double xp[3] = {x[0], x[1], 0}, xm[3] = {x[0], x[1], 0};
      xp[d] += 1e-6;
      xm[d] -= 1e-6;
      double fp[10] = {}, fm[10] = {};
      fmr::fems::elem::calc_poly_func_T2D10N(fp, xp);
      fmr::fems::elem::calc_poly_func_T2D10N(fm, xm);
      double sum = 0;
      for (int i=0; i<10; ++i) {
        EXPECT_DOUBLE_EQ(g[2*i+d], gt[d*10+i]);
        EXPECT_NEAR(g[2*i+d], (fp[i]-fm[i])/2e-6, 1e-8);
        sum += g[2*i+d];
      }
      EXPECT_NEAR(sum, 0, 1e-13);
    }
    for (int a=0; a<=3; ++a) for (int b=0; b<=3-a; ++b) {
      double value = 0, dx = 0, dy = 0;
      for (int i=0; i<10; ++i) {
        const double nodal = std::pow(nodes[i][0], a)*std::pow(nodes[i][1], b);
        value += f[i]*nodal;
        dx += g[2*i]*nodal;
        dy += g[2*i+1]*nodal;
      }
      EXPECT_NEAR(value, std::pow(x[0],a)*std::pow(x[1],b), 1e-13);
      EXPECT_NEAR(dx, a ? a*std::pow(x[0],a-1)*std::pow(x[1],b) : 0, 1e-13);
      EXPECT_NEAR(dy, b ? b*std::pow(x[0],a)*std::pow(x[1],b-1) : 0, 1e-13);
    }
  }
}

TEST(FmrElemPoly, CubicTriangleConstantLoadIntegration) {
  const double points[4][3] = {
    {1.0/3,1.0/3,0}, {0.2,0.2,0}, {0.6,0.2,0}, {0.2,0.6,0}
  };
  const double weights[4] = {-27.0/96,25.0/96,25.0/96,25.0/96};
  double loads[10] = {};
  for (int q=0; q<4; ++q) {
    double f[10] = {};
    fmr::fems::elem::calc_poly_func_T2D10N(f, points[q]);
    for (int i=0; i<10; ++i) loads[i] += weights[q]*f[i];
  }

  for (int i=0; i<10; ++i) {
    EXPECT_NEAR(loads[i], i<3 ? 1.0/60 : (i<9 ? 3.0/80 : 9.0/40), 1e-14);
  }
}

TEST(FmrElemPoly, CubicTriangleMatchesTetrahedronFace) {
  const double points[3][3] = {{0.1,0.2,0}, {0.4,0.3,0}, {0.0,0.7,0}};
  const int face_nodes[10] = {0,1,2,4,5,6,7,8,9,16};
  for (const auto& x : points) {
    double triangle[10] = {}, tri_grad[20] = {};
    double tetrahedron[20] = {}, tet_grad[60] = {};
    fmr::fems::elem::calc_poly_func_T2D10N(triangle, x);
    fmr::fems::elem::calc_poly_grad_node_T2D10N(tri_grad, x);
    femera::grid::elem::Tets::calc_poly_func_20(tetrahedron, x);
    femera::grid::elem::Tets::calc_poly_grad_node_20(tet_grad, x);
    for (int i=0; i<10; ++i) {
      EXPECT_NEAR(triangle[i], tetrahedron[face_nodes[i]], 1e-14);
      for (int d=0; d<2; ++d) {
        EXPECT_NEAR(tri_grad[2*i+d], tet_grad[d*20+face_nodes[i]], 1e-14);
      }
    }
  }
}

TEST(FmrElemPoly, IncompleteCubicTriangleBasisAndGradients) {
  const double nodes[9][3] = {
    {0,0,0}, {1,0,0}, {0,1,0},
    {1.0/3,0,0}, {2.0/3,0,0},
    {2.0/3,1.0/3,0}, {1.0/3,2.0/3,0},
    {0,2.0/3,0}, {0,1.0/3,0}
  };
  for (int n=0; n<9; ++n) {
    double guarded[11] = {};
    guarded[0] = 123;
    guarded[10] = 456;
    ASSERT_EQ(fmr::fems::elem::calc_poly_func_T2D9N(guarded+1, nodes[n]), guarded+1);
    EXPECT_DOUBLE_EQ(guarded[0], 123);
    EXPECT_DOUBLE_EQ(guarded[10], 456);
    for (int i=0; i<9; ++i) EXPECT_NEAR(guarded[i+1], i==n ? 1.0 : 0.0, 1e-14);
  }
  const double points[4][3] = {
    {0.17,0.21,0}, {0.43,0.13,0}, {0.0,0.5,0}, {1.0/3,1.0/3,0}
  };
  for (const auto& x : points) {
    double f[9] = {}, g_storage[20] = {}, gt_storage[20] = {};
    g_storage[0] = gt_storage[0] = 123;
    g_storage[19] = gt_storage[19] = 456;
    double* g = g_storage+1;
    double* gt = gt_storage+1;
    fmr::fems::elem::calc_poly_func_T2D9N(f, x);
    ASSERT_EQ(fmr::fems::elem::calc_poly_grad_node_T2D9N(g, x), g);
    ASSERT_EQ(fmr::fems::elem::calc_poly_node_grad_T2D9N(gt, x), gt);
    EXPECT_DOUBLE_EQ(g_storage[0], 123);
    EXPECT_DOUBLE_EQ(g_storage[19], 456);
    EXPECT_DOUBLE_EQ(gt_storage[0], 123);
    EXPECT_DOUBLE_EQ(gt_storage[19], 456);
    for (int d=0; d<2; ++d) {
      double xp[3] = {x[0], x[1], 0}, xm[3] = {x[0], x[1], 0};
      xp[d] += 1e-6;
      xm[d] -= 1e-6;
      double fp[9] = {}, fm[9] = {}, sum = 0;
      fmr::fems::elem::calc_poly_func_T2D9N(fp, xp);
      fmr::fems::elem::calc_poly_func_T2D9N(fm, xm);
      for (int i=0; i<9; ++i) {
        EXPECT_DOUBLE_EQ(g[2*i+d], gt[d*9+i]);
        EXPECT_NEAR(g[2*i+d], (fp[i]-fm[i])/2e-6, 1e-8);
        sum += g[2*i+d];
      }
      EXPECT_NEAR(sum, 0, 1e-13);
    }
    for (int a=0; a<=2; ++a) for (int b=0; b<=2-a; ++b) {
      double value = 0, dx = 0, dy = 0;
      for (int i=0; i<9; ++i) {
        const double nodal = std::pow(nodes[i][0], a)*std::pow(nodes[i][1], b);
        value += f[i]*nodal;
        dx += g[2*i]*nodal;
        dy += g[2*i+1]*nodal;
      }
      EXPECT_NEAR(value, std::pow(x[0],a)*std::pow(x[1],b), 1e-13);
      EXPECT_NEAR(dx, a ? a*std::pow(x[0],a-1)*std::pow(x[1],b) : 0, 1e-13);
      EXPECT_NEAR(dy, b ? b*std::pow(x[0],a)*std::pow(x[1],b-1) : 0, 1e-13);
    }
  }
}

TEST(FmrElemPoly, IncompleteTriangleMatchesCubicEdgesAndConstrainedTri10) {
  const double vertices[3][2] = {{0,0}, {1,0}, {0,1}};
  const double parameters[] = {0.17, 0.5, 0.83};
  for (int edge=0; edge<3; ++edge) for (const double t : parameters) {
    const int next = (edge+1)%3;
    const double x[3] = {
      (1-t)*vertices[edge][0]+t*vertices[next][0],
      (1-t)*vertices[edge][1]+t*vertices[next][1], 0
    };
    const double xi[3] = {t,0,0};
    double tri[9] = {}, grad[18] = {}, bar[4] = {}, dbar[4] = {};
    fmr::fems::elem::calc_poly_func_T2D9N(tri, x);
    fmr::fems::elem::calc_poly_grad_node_T2D9N(grad, x);
    fmr::fems::elem::calc_poly_func_C1D4N(bar, xi);
    fmr::fems::elem::calc_poly_grad_node_C1D4N(dbar, xi);
    const int indices[4] = {edge,next,3+2*edge,4+2*edge};
    for (int i=0; i<9; ++i) {
      int mapped = -1;
      for (int j=0; j<4; ++j) if (indices[j] == i) mapped = j;
      const double tangent = grad[2*i]*(vertices[next][0]-vertices[edge][0])
        + grad[2*i+1]*(vertices[next][1]-vertices[edge][1]);
      EXPECT_NEAR(tri[i], mapped<0 ? 0 : bar[mapped], 1e-14);
      EXPECT_NEAR(tangent, mapped<0 ? 0 : dbar[mapped], 1e-13);
    }
  }
  const double points[3][3] = {{0.13,0.21,0}, {0.5,0.2,0}, {1.0/3,1.0/3,0}};
  const double nodal[9] = {0.7,-1.1,2.0,0.2,3.0,-0.5,1.7,-2.0,0.3};
  double center = 0;
  for (int i=0; i<9; ++i) center += (i<3 ? -1.0/6 : 1.0/4)*nodal[i];
  for (const auto& x : points) {
    double f9[9] = {}, f10[10] = {}, g9[18] = {}, g10[20] = {};
    fmr::fems::elem::calc_poly_func_T2D9N(f9, x);
    fmr::fems::elem::calc_poly_func_T2D10N(f10, x);
    fmr::fems::elem::calc_poly_grad_node_T2D9N(g9, x);
    fmr::fems::elem::calc_poly_grad_node_T2D10N(g10, x);
    double v9 = 0, v10 = center*f10[9];
    double d9[2] = {}, d10[2] = {center*g10[18],center*g10[19]};
    for (int i=0; i<9; ++i) {
      v9 += nodal[i]*f9[i];
      v10 += nodal[i]*f10[i];
      for (int d=0; d<2; ++d) {
        d9[d] += nodal[i]*g9[2*i+d];
        d10[d] += nodal[i]*g10[2*i+d];
      }
    }
    EXPECT_NEAR(v9, v10, 1e-14);
    for (int d=0; d<2; ++d) EXPECT_NEAR(d9[d], d10[d], 1e-13);
  }
}

TEST(FmrElemPoly, IncompleteTriangleConstantLoadIntegration) {
  const double points[4][3] = {
    {1.0/3,1.0/3,0}, {0.2,0.2,0}, {0.6,0.2,0}, {0.2,0.6,0}
  };
  const double weights[4] = {-27.0/96,25.0/96,25.0/96,25.0/96};
  double loads[9] = {};
  for (int q=0; q<4; ++q) {
    double f[9] = {};
    fmr::fems::elem::calc_poly_func_T2D9N(f, points[q]);
    for (int i=0; i<9; ++i) loads[i] += weights[q]*f[i];
  }
  for (int i=0; i<9; ++i) {
    EXPECT_NEAR(loads[i], i<3 ? -1.0/48 : 3.0/32, 1e-14);
  }
}

TEST(FmrElemPoly, IncompleteTriangleIsSymmetricUnderVertexPermutations) {
  const int permutations[6][3] = {
    {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
  };
  const double bary[9][3] = {
    {1,0,0}, {0,1,0}, {0,0,1},
    {2.0/3,1.0/3,0}, {1.0/3,2.0/3,0},
    {0,2.0/3,1.0/3}, {0,1.0/3,2.0/3},
    {1.0/3,0,2.0/3}, {2.0/3,0,1.0/3}
  };
  const double lambda[3] = {0.26,0.31,0.43};
  const double x[3] = {lambda[1], lambda[2], 0};
  double original[9] = {};
  fmr::fems::elem::calc_poly_func_T2D9N(original, x);
  for (const auto& perm : permutations) {
    const double xp[3] = {lambda[perm[1]], lambda[perm[2]], 0};
    double transformed[9] = {};
    fmr::fems::elem::calc_poly_func_T2D9N(transformed, xp);
    for (int i=0; i<9; ++i) {
      int mapped = -1;
      for (int j=0; j<9; ++j) {
        bool match = true;
        for (int k=0; k<3; ++k) {
          if (std::abs(bary[j][k]-bary[i][perm[k]]) > 1e-14) match = false;
        }
        if (match) mapped = j;
      }
      ASSERT_GE(mapped, 0);
      EXPECT_NEAR(original[i], transformed[mapped], 1e-14);
    }
  }
}
