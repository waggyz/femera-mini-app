#include "../../femera.hpp"
#include "../Elem.test.hpp"
#include "Quad.hpp"

#include <gtest/gtest.h>

#define DEBUG_PRINT
#ifdef DEBUG_PRINT
//#include "../../fmr.hpp"
#endif

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}

TEST( GridElemQuad, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

namespace femera { namespace grid { namespace elem {

TEST( GridCellElemQuad, QuadMetadata ){
  EXPECT_EQ( Quad::get_node_n (), 4 );
}

TEST( GridCellElemQuad, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (
    &Quad::vert_coor [3* 0],
    &Quad::vert_coor [3* 1],
    &Quad::vert_coor [3* 2]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &Quad::vert_coor [3* 2],
    &Quad::vert_coor [3* 3],
    &Quad::vert_coor [3* 0]), "+z" );
}
TEST( GridCellElemQuad, ShapeGradient ){
  const fmr::Local_int elem_d = Quad::get_elem_d ();
  const fmr::Local_int node_n = Quad::get_node_n ();
  const double eval_coor[elem_d] = {0.5, 0.5};// right in the middle
  double dN_node_grad[node_n*elem_d] = {0.0,0.0, 0.0,0.0, 0.0,0.0, 0.0,0.0};
  double dN_grad_node[elem_d*node_n] = {0.0,0.0,0.0,0.0, 0.0,0.0,0.0,0.0};
  Quad::get_poly_node_grad (dN_node_grad, eval_coor);
  Quad::get_poly_grad_node (dN_grad_node, eval_coor);

  for (fmr::Local_int i = 0; i < elem_d; ++i) {// Check these are transposes.
    for (fmr::Local_int j = 0; j < node_n; ++j) {
      //EXPECT_DOUBLE_EQ(dN_node_grad[elem_d*j + i], dN_grad_node[node_n*i + j]);
      EXPECT_DOUBLE_EQ(dN_grad_node[elem_d*j + i], dN_node_grad[node_n*i + j]);
    }
  }

#ifdef DEBUG_PRINT
  for (fmr::Local_int i=0; i<node_n; ++i){
    mini->data->send (fmr::out, "elem", "quad", "grad", "%f,%f",
      dN_node_grad[elem_d*i + 0], dN_node_grad[elem_d*i + 1]);
  }
  for (fmr::Local_int i=0; i<elem_d; ++i){
    mini->data->send (fmr::out, "elem", "quad", "graT", "%f,%f,%f,%f",
      dN_grad_node[node_n*i + 0], dN_grad_node[node_n*i + 1],
      dN_grad_node[node_n*i + 2], dN_grad_node[node_n*i + 3]);
  }
#endif
  const double node_vals[node_n*elem_d]
   = {0.0,0.0,
      1.0,0.0,// function of x only
      1.0,0.0,
      0.0,0.0};
  double grad_expc[elem_d] = {1.0, 0.0}; // expected gradient for function of x only  
  double grad[elem_d];// correctnes check


#define IS_CORRECT_LAYOUT

  for (fmr::Local_int i = 0; i < elem_d; ++i) {
    grad[i] = 0.0;
    for (fmr::Local_int j = 0; j < node_n; ++j) {
#ifdef IS_CORRECT_LAYOUT
      grad[i] += dN_node_grad[2*j + i] * node_vals[2*j + i];
#else
      grad[i] += dN_node_grad[4*i + j] * node_vals[2*j + i];//works w/ bad layout
#endif
    }
  }
  for (fmr::Local_int i = 0; i < elem_d; ++i) {
    EXPECT_DOUBLE_EQ(grad[i], grad_expc[i]);
  }
  for (fmr::Local_int i = 0; i < elem_d; ++i) {
    grad[i] = 0.0;
    for (fmr::Local_int j = 0; j < node_n; ++j) {
#ifdef IS_CORRECT_LAYOUT
      grad[i] += dN_grad_node[4*i + j] * node_vals[2*j + i];
#else
      grad[i] += dN_grad_node[2*j + i] * node_vals[2*j + i];//works w/ bad layout
#endif
    }
  }
  for (fmr::Local_int i = 0; i < elem_d; ++i) {
    EXPECT_DOUBLE_EQ(grad[i], grad_expc[i]);
  }
#undef IS_CORRECT_LAYOUT


}

} } }//end femera::grid::elem:: namespace

#undef DEBUG_PRINT