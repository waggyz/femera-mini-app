#include "../femera.hpp"
#include "Logs.hpp"

#include "gtest/gtest.h"

auto mini = femera::new_sims ();
fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
namespace femera { namespace test {

  const auto csv_123 = femera::data::Logs::data_line<int,uint,char> (1,2,3);
  const auto csv_456 = femera::data::Logs::data_line (4,5,6);
  const auto csv_hello1 =femera::data::Logs::data_line(std::string ("hello"),1);
  const auto csv_hello2
    = femera::data::Logs::data_line<std::string,int> ("hello",2);
  const auto csv_hello3 = femera::data::Logs::data_line ("hello",3);
  const auto csv_hello4bye = femera::data::Logs::data_line ("hello",4,"bye");
  const auto csv_1int = femera::data::Logs::data_line (1);
  const auto csv_1str = femera::data::Logs::data_line ("one");

  TEST( Logs, TrivialTest)  { EXPECT_EQ( 0, 0 ); }
  TEST( Logs, CSV123)       { EXPECT_EQ( csv_123,       "1,2,3"); }
  TEST( Logs, CSV456)       { EXPECT_EQ( csv_456,       "4,5,6"); }
  TEST( Logs, CSVHello1)    { EXPECT_EQ( csv_hello1,    "\"hello\",1"); }
  TEST( Logs, CSVHello2)    { EXPECT_EQ( csv_hello2,    "\"hello\",2"); }
  TEST( Logs, CSVHello3)    { EXPECT_EQ( csv_hello3,    "\"hello\",3"); }
  TEST( Logs, CSVHello4bye) { EXPECT_EQ( csv_hello4bye, "\"hello\",4,\"bye\"");}
  TEST( Logs, CSV1int)      { EXPECT_EQ( csv_1int,      "1"); }
  TEST( Logs, CSV1Str)      { EXPECT_EQ( csv_1str,      "\"one\""); }

} }//end femerea::test:: namespace
