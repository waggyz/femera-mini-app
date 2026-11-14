#include "elem-jacs.hpp"

#include <gtest/gtest.h>

TEST(FmrElem, JacobianCalculationsMatchExpectedResults) {
  constexpr fmr::Local_int node_n = 3;
  constexpr fmr::Local_int intp_n = 2;

  for(fmr::Dim_int sims_d=1; sims_d<=3; ++sims_d){
    SCOPED_TRACE(sims_d);
    for(fmr::Dim_int elem_d=1; elem_d<=sims_d; ++elem_d){
      SCOPED_TRACE(elem_d);
      double node_coor[node_n * 3] = {};
      double coor_node[node_n * 3] = {};
      double poly_grad[intp_n * node_n * 3] = {};
      double poly_grad_ref_major[intp_n * node_n * 3] = {};
      double jacs_aos[intp_n * 3 * 3] = {};
      double jacs_soa[intp_n * 3 * 3] = {};
      double jacs_ref_major[intp_n * 3 * 3] = {};

      for(fmr::Local_int k=0; k<node_n; ++k){
        for(fmr::Dim_int i=0; i<sims_d; ++i){
          const double coordinate = 2.0 + 3.0*k + 2.0*i + k*i;
          node_coor[k*sims_d + i] = coordinate;
          coor_node[i*node_n + k] = coordinate;
        }
      }

      for(fmr::Local_int ip=0; ip<intp_n; ++ip){
        for(fmr::Local_int k=0; k<node_n; ++k){
          for(fmr::Dim_int j=0; j<elem_d; ++j){
            const double gradient = (ip + 1.0)*(k + 1.0) - 2.0*j;
            poly_grad[ip*node_n*elem_d + k*elem_d + j] = gradient;
            poly_grad_ref_major[ip*elem_d*node_n + j*node_n + k] = gradient;
          }
        }
      }

      fmr::fems::elem::elem_jacs_node_coor_ref(
        jacs_aos, node_coor, poly_grad, intp_n, node_n, sims_d, elem_d);
      fmr::fems::elem::elem_jacs_coor_node_ref(
        jacs_soa, coor_node, poly_grad, intp_n, node_n, sims_d, elem_d);
      fmr::fems::elem::elem_jacs_node_coor_a(
        jacs_ref_major, node_coor, poly_grad_ref_major, intp_n, node_n,
        sims_d, elem_d);

      for(fmr::Local_int ip=0; ip<intp_n; ++ip){
        for(fmr::Dim_int i=0; i<sims_d; ++i){
          for(fmr::Dim_int j=0; j<elem_d; ++j){
            double expected = 0.0;
            for(fmr::Local_int k=0; k<node_n; ++k){
              const double coordinate = 2.0 + 3.0*k + 2.0*i + k*i;
              const double gradient = (ip + 1.0)*(k + 1.0) - 2.0*j;
              expected += coordinate*gradient;
            }
            const fmr::Local_int index
              = ip*sims_d*elem_d + i*elem_d + j;
            EXPECT_DOUBLE_EQ(jacs_aos[index], expected);
            EXPECT_DOUBLE_EQ(jacs_soa[index], expected);
            EXPECT_DOUBLE_EQ(jacs_ref_major[index], expected);
          }
        }
      }
    }
  }
}

TEST(FmrElem, SurfaceEmbeddedInThreeDimensionsHasRectangularJacobian) {
  const double node_coor[9] = {0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 3.0, 4.0};
  const double coor_node[9] = {0.0, 2.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 4.0};
  const double gradients[6] = {-1.0, -1.0, 1.0, 0.0, 0.0, 1.0};
  const double gradients_ref_major[6] = {-1.0, 1.0, 0.0, -1.0, 0.0, 1.0};
  const double expected[6] = {2.0, 0.0, 0.0, 3.0, 0.0, 4.0};
  double jac_aos[7] = {};
  double jac_soa[7] = {};
  double jac_ref_major[7] = {};
  jac_aos[6] = jac_soa[6] = jac_ref_major[6] = -99.0;

  fmr::fems::elem::elem_jacs_node_coor_ref(
    jac_aos, node_coor, gradients, 1u, 3u, 3u, 2u);
  fmr::fems::elem::elem_jacs_coor_node_ref(
    jac_soa, coor_node, gradients, 1u, 3u, 3u, 2u);
  fmr::fems::elem::elem_jacs_node_coor_a(
    jac_ref_major, node_coor, gradients_ref_major, 1u, 3u, 3u, 2u);

  for(fmr::Local_int i=0; i<6; ++i){
    EXPECT_DOUBLE_EQ(jac_aos[i], expected[i]);
    EXPECT_DOUBLE_EQ(jac_soa[i], expected[i]);
    EXPECT_DOUBLE_EQ(jac_ref_major[i], expected[i]);
  }
  EXPECT_DOUBLE_EQ(jac_aos[6], -99.0);
  EXPECT_DOUBLE_EQ(jac_soa[6], -99.0);
  EXPECT_DOUBLE_EQ(jac_ref_major[6], -99.0);
}

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
