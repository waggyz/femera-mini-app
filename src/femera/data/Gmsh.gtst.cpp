#include "../femera.hpp"
#include "Gmsh.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
namespace femera { namespace test {

  TEST( Gmsh, TrivialTest)  { EXPECT_EQ( 0, 0 ); }

} }//end femerea::test:: namespace
