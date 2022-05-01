#include "femera.hpp"
#include "Bulk.hpp"

#include "gtest/gtest.h"

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  TEST(Bulk, IntSizes) {
    EXPECT_EQ( 0, 0);
  }
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
