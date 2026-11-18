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
  EXPECT_EQ( Elem<Ipin>::get_poly_type(), fmr::math::Poly::None );
  EXPECT_EQ( Elem<Ipin>::get_elem_d(), 1 );
  EXPECT_EQ( Elem<Ipin>::get_vert_n(), 2 );
}
TEST( GridElemIpin, NoPolyFuncGrad ){
  const auto poly_size = Elem<Ipin>::get_poly_func_size();
  const auto grad_size = Elem<Ipin>::get_poly_grad_size();
  EXPECT_EQ( poly_size, 0 );
  EXPECT_EQ( grad_size, 0 );
  fmr::Phys_float out_buf[1] = {42};
  const fmr::Phys_float intp[1] = {};
  EXPECT_FALSE( Elem<Ipin>::get_poly_func_vals(out_buf,intp) );
  EXPECT_FALSE( Elem<Ipin>::get_poly_grad_node(out_buf,intp) );
  EXPECT_FALSE( Elem<Ipin>::get_poly_node_grad(out_buf,intp) );
  EXPECT_DOUBLE_EQ( out_buf[0], 42 );
}
//
} } }//end femera::grid::elem:: namespace
  