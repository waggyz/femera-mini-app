#include "../femera.hpp"
#include "Logs.hpp"

#include "gtest/gtest.h"

auto mini = fmr::new_sims ();

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
  const auto csv_1char = femera::data::Logs::data_line ('c');// integer type
  const auto csv_fp12 = femera::data::Logs::data_line (float(1),double(2));

  TEST( Logs, TrivialTest)  { EXPECT_EQ( 0, 0 ); }
  TEST( Logs, CSV123)       { EXPECT_EQ( csv_123,       "1,2,3"); }
  TEST( Logs, CSV456)       { EXPECT_EQ( csv_456,       "4,5,6"); }
  TEST( Logs, CSVHello1)    { EXPECT_EQ( csv_hello1,    "\"hello\",1"); }
  TEST( Logs, CSVHello2)    { EXPECT_EQ( csv_hello2,    "\"hello\",2"); }
  TEST( Logs, CSVHello3)    { EXPECT_EQ( csv_hello3,    "\"hello\",3"); }
  TEST( Logs, CSVHello4bye) { EXPECT_EQ( csv_hello4bye, "\"hello\",4,\"bye\"");}
  TEST( Logs, CSV1int)      { EXPECT_EQ( csv_1int,      "1"); }
  TEST( Logs, CSV1Str)      { EXPECT_EQ( csv_1str,      "\"one\""); }
  TEST( Logs, CSV1char)     { EXPECT_EQ( csv_1char,     "99"); }// integer type
  TEST( Logs, CSVfp12)      { EXPECT_EQ( csv_fp12,
    "1.0000000e+00,2.000000000000000E+00"); }

} }//end femerea::test:: namespace
