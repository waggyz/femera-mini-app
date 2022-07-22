#include "../femera.hpp"
#include "Pets.hpp"

#include <gtest/gtest.h>

#include <stdlib.h>          // atof

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  inline
  double get_petsc_version () {
    const auto D = mini->data->get_task (femera::Task_type::Petsc);
    std::string ver = "-1.0";
    if (D != nullptr) {};//TODO ver = D->get_version ();}
    return atof (ver.c_str());
  }
  TEST( PETSc, TrivialTest) { EXPECT_EQ( 0, 0 ); }
//  TODO_TEST( PETSc, VersionTODO)     { EXPECT_GE( get_petsc_version(), -1.0 ); }
  //
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}
