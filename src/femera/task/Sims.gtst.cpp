#include "../femera.hpp"

#include <gtest/gtest.h>


namespace femera { namespace test {
  //
  auto mini = fmr::new_jobs ();
  //
  TEST( Sims, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
  } }// end femera::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}
