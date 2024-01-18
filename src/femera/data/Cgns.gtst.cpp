#include "../femera.hpp"
#include "Cgns.hpp"

#include <gtest/gtest.h>

#include <stdlib.h>          // atof

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  inline
  double get_cgns_version () {
    const auto D = mini->data->get_task (femera::Task_type::Cgns);
    std::string ver = "-1.0";
    if (D != nullptr) {ver = D->get_version ();}
    return atof (ver.c_str());
  }
  TEST( Cgns, TrivialTest) { EXPECT_EQ( 0, 0 ); }
  TEST( Cgns, Version)     { EXPECT_GE( get_cgns_version (), 4.0 ); }//WAS 5.0
  //
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}
