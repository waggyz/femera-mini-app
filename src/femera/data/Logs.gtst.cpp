#include "../femera.hpp"
#include "Logs.hpp"

#include "gtest/gtest.h"

auto mini = femera::new_sims ();
fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
namespace femera { namespace test {
const auto csv123 = femera::data::Logs::data_line<int,int,int>("",1,2,3);
const auto csv456 = femera::data::Logs::data_line<int,int,int>(4,5,6);

TEST( Logs, TrivialTest ){
  EXPECT_EQ( 0, 0 );
}
TEST(Logs, DataLine123) {
  EXPECT_EQ( csv123, "1,2,3");
}
TEST(Logs, DataLine456) {
  EXPECT_EQ( csv456, "4,5,6");
}
} }//end femerea::test:: namespace
