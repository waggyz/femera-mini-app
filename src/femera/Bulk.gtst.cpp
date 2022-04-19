#include "femera.hpp"
#include "Bulk.hpp"

#include "gtest/gtest.h"

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  auto bulk = femera::data::Bulk();
  const auto ints10 = std::string("test-10-ints");
  //
  TEST(Bulk, Ints10) {
    EXPECT_EQ( bulk.add (ints10,10,1)[9], 1);
    //XXX_EXPECT_EQ( bulk.get (ints10<int>)[5], 1);
  }
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
