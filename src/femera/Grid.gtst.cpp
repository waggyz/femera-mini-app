#include "Grid.hpp"

#include <gtest/gtest.h>

#if 0

// Use this for concrete classes.

#include "femera.hpp"
#include "data/Vals.hpp"

//...

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  auto vals = femera::data::Vals ();

//...

  TEST(Mtrl, IsOK) {
    EXPECT_EQ( 1, 1);
  }

//...

} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}
#else
TEST(Grid, IsOK) {
  EXPECT_EQ( 1, 1);
}
int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
#endif
