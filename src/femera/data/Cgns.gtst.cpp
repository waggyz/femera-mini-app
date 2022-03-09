#include "../femera.hpp"
#include "Cgns.hpp"

#include "gtest/gtest.h"
#include <stdlib.h>

auto mini = fmr::new_sims ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
namespace femera { namespace test {
  inline
  double get_cgns_version () {
    auto D = mini->data->get_task (femera::Plug_type::Cgns);
    std::string ver = "-1.0";
    if (D != nullptr) {
      ver = D->get_version ();
    }
    return atof(ver.c_str());
  }

  TEST( Cgns, TrivialTest)  { EXPECT_EQ( 0, 0 ); }
  TEST( Cgns, Version)  { EXPECT_GE( get_cgns_version(), 5.0 ); }

} }//end femerea::test:: namespace
