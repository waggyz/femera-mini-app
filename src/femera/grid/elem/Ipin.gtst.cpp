#include "../../femera.hpp"
#include "Ipin.hpp"

#include <gtest/gtest.h>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}

namespace femera { namespace grid { namespace elem {

TEST( GridElemIpin, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
TEST( GridElemIpin, ElemMetaData ){
  EXPECT_EQ( Elem<Ipin>::get_poly_type(), fmr::math::Poly::Full );
  EXPECT_EQ( Elem<Ipin>::get_elem_d(), 1 );
  EXPECT_EQ( Elem<Ipin>::get_vert_n(), 2 );
  EXPECT_EQ( Elem<Ipin>::get_edge_n(), 1 );
  EXPECT_DOUBLE_EQ( Elem<Ipin>::get_edge_l(), 1.0 );
}
TEST( GridElemIpin, P1ShapeFunctionsAndGradients ){
  const auto poly_size = Elem<Ipin>::get_poly_func_size();
  const auto grad_size = Elem<Ipin>::get_poly_grad_size();
  EXPECT_EQ( poly_size, 2 );
  EXPECT_EQ( grad_size, 2 );

  const fmr::Phys_float intp[3] = {0.0, 0.5, 1.0};
  fmr::Phys_float values[6] = {};
  ASSERT_TRUE(Elem<Ipin>::get_poly_func_vals(values, intp, 2, 3, 1));
  EXPECT_DOUBLE_EQ(values[0], 1.0);
  EXPECT_DOUBLE_EQ(values[1], 0.0);
  EXPECT_DOUBLE_EQ(values[2], 0.5);
  EXPECT_DOUBLE_EQ(values[3], 0.5);
  EXPECT_DOUBLE_EQ(values[4], 0.0);
  EXPECT_DOUBLE_EQ(values[5], 1.0);

  const fmr::Phys_float grad_point[1] = {0.25};
  fmr::Phys_float grad_node[2] = {};
  fmr::Phys_float node_grad[2] = {};
  ASSERT_TRUE(Elem<Ipin>::get_poly_grad_node(grad_node, grad_point));
  ASSERT_TRUE(Elem<Ipin>::get_poly_node_grad(node_grad, grad_point));
  EXPECT_DOUBLE_EQ(grad_node[0], -1.0);
  EXPECT_DOUBLE_EQ(grad_node[1], 1.0);
  EXPECT_DOUBLE_EQ(node_grad[0], -1.0);
  EXPECT_DOUBLE_EQ(node_grad[1], 1.0);
}
TEST( GridElemIpin, ReferenceGeometryAndIntegrationRule ){
  const Elem<Ipin> ipin;
  const auto* bars = ipin.get_bars_vert_conn();
  ASSERT_NE(bars, nullptr);
  EXPECT_EQ(bars[0], 0);
  EXPECT_EQ(bars[1], 1);

  const auto* vert_coor = ipin.get_elem_vert_coor();
  const auto* coor_vert = ipin.get_elem_coor_vert();
  ASSERT_NE(vert_coor, nullptr);
  ASSERT_NE(coor_vert, nullptr);
  EXPECT_DOUBLE_EQ(vert_coor[0], 0.0);
  EXPECT_DOUBLE_EQ(vert_coor[3], 1.0);
  EXPECT_DOUBLE_EQ(coor_vert[0], 0.0);
  EXPECT_DOUBLE_EQ(coor_vert[1], 1.0);

  EXPECT_EQ(Ipin::intg_1_n, 1);
  EXPECT_DOUBLE_EQ(Ipin::intg_1_ptwt[0], 0.5);
  EXPECT_DOUBLE_EQ(Ipin::intg_1_ptwt[1], 1.0);
}
//
} } }//end femera::grid::elem:: namespace
  