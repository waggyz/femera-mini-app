#include "../../femera.hpp"
#include "Pins.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}

namespace femera { namespace grid { namespace elem {

TEST( GridElemPins, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
TEST( GridElemPins, ElemMetaData ){
  EXPECT_EQ( Elem<Pins>::get_poly_type(), fmr::math::Poly::None );
  EXPECT_EQ( Elem<Pins>::get_elem_d(), 1 );
  EXPECT_EQ( Elem<Pins>::get_vert_n(), 1 );
}

} } }
