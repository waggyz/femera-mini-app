#include "elem-jacs.hpp"

#include <gtest/gtest.h>

TEST(FmrElem, JacobianCalculationsMatchExpectedResults) {
  constexpr fmr::Local_int node_n = 3;
  constexpr fmr::Local_int intp_n = 2;

  for(fmr::Dim_int sims_d=1; sims_d<=3; ++sims_d){
    SCOPED_TRACE(sims_d);

    double node_coor[node_n * 3] = {};// node-major: node, dimension
    double coor_node[node_n * 3] = {};// transposed: dimension, node
    double poly_grad[intp_n * node_n * 3] = {};// node-major: node, dimension
    double poly_grat[intp_n * node_n * 3] = {};// transposed: dimension, node
    double jacs_caos[intp_n * 3 * 3] = {};// node-major: node, dimension
    double jacs_csoa[intp_n * 3 * 3] = {};// transposed: dimension, node, integration point
    double jacs_node_coor_a[intp_n * 3 * 3] = {};// node-major: node, dimension, integration point

    for(fmr::Local_int k=0; k<node_n; ++k){
      for(fmr::Local_int i=0; i<sims_d; ++i){
        const double coordinate = 2.0 + 3.0*k + 2.0*i + k*i;
        node_coor[k*sims_d + i] = coordinate;
        coor_node[i*node_n + k] = coordinate;
      }
    }

    for(fmr::Local_int ip=0; ip<intp_n; ++ip){
      for(fmr::Local_int k=0; k<node_n; ++k){
        for(fmr::Local_int j=0; j<sims_d; ++j){
          const double gradient = (ip + 1.0)*(k + 1.0) - 2.0*j;
          poly_grad[ip*node_n*sims_d + k*sims_d + j] = gradient;
          poly_grat[ip*node_n*sims_d + j*node_n + k] = gradient;
        }
      }
    }

    fmr::fems::elem::elem_jacs_node_coor_ref(
      jacs_caos, node_coor, poly_grad, intp_n, node_n, sims_d);
    fmr::fems::elem::elem_jacs_coor_node_ref(
      jacs_csoa, coor_node, poly_grad, intp_n, node_n, sims_d);
    fmr::fems::elem::elem_jacs_node_coor_a(
      jacs_node_coor_a, node_coor, poly_grat, intp_n, node_n, sims_d);

    for(fmr::Local_int ip=0; ip<intp_n; ++ip){
      for(fmr::Local_int i=0; i<sims_d; ++i){
        for(fmr::Local_int j=0; j<sims_d; ++j){
          double expected = 0.0;
          for(fmr::Local_int k=0; k<node_n; ++k){
            const double coordinate = 2.0 + 3.0*k + 2.0*i + k*i;
            const double gradient = (ip + 1.0)*(k + 1.0) - 2.0*j;
            expected += coordinate*gradient;
          }

          const fmr::Local_int index = ip*sims_d*sims_d + i*sims_d + j;
          EXPECT_DOUBLE_EQ(jacs_caos[index], expected);
          EXPECT_DOUBLE_EQ(jacs_csoa[index], expected);
          EXPECT_DOUBLE_EQ(jacs_node_coor_a[index], expected);
        }
      }
    }
  }
}

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
