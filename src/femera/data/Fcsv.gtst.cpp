#include "../femera.hpp"
#include "Fcsv.hpp"

#include "gtest/gtest.h"

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc, argv));
}
namespace femera { namespace test {

  const auto csv_123 = femera::data::Fcsv::data_line<int,uint,char> (1,2,3);
  const auto csv_456 = femera::data::Fcsv::data_line (4,5,6);
  const auto csv_hello1 =femera::data::Fcsv::data_line(std::string ("hello"),1);
  const auto csv_hello2
    = femera::data::Fcsv::data_line<std::string,int> ("hello",2);
  const auto csv_hello3 = femera::data::Fcsv::data_line ("hello",3);
  const auto csv_hello4bye = femera::data::Fcsv::data_line ("hello",4,"bye");
  const auto csv_1int = femera::data::Fcsv::data_line (1);
  const auto csv_1str = femera::data::Fcsv::data_line ("one");
  const auto csv_1char = femera::data::Fcsv::data_line ('c');// integer type
  //const auto csv_1cstr = femera::data::Fcsv::data_line<std::string> ('c');
  // The above does not work; char type is treated as an integer.
  const auto csv_fp12 = femera::data::Fcsv::data_line (float(1),double(2));
  const auto csv_ff12
    = femera::data::Fcsv::data_line<float,float> (float(1),2);

  TEST( Fcsv, TrivialTest)  { EXPECT_EQ( 0, 0 ); }
  TEST( Fcsv, CSV123)       { EXPECT_EQ( csv_123,       "1,2,3"); }
  TEST( Fcsv, CSV456)       { EXPECT_EQ( csv_456,       "4,5,6"); }
  TEST( Fcsv, CSVHello1)    { EXPECT_EQ( csv_hello1,    "\"hello\",1"); }
  TEST( Fcsv, CSVHello2)    { EXPECT_EQ( csv_hello2,    "\"hello\",2"); }
  TEST( Fcsv, CSVHello3)    { EXPECT_EQ( csv_hello3,    "\"hello\",3"); }
  TEST( Fcsv, CSVHello4bye) { EXPECT_EQ( csv_hello4bye, "\"hello\",4,\"bye\"");}
  TEST( Fcsv, CSV1int)      { EXPECT_EQ( csv_1int,      "1"); }
  TEST( Fcsv, CSV1Str)      { EXPECT_EQ( csv_1str,      "\"one\""); }
  TEST( Fcsv, CSV1char)     { EXPECT_EQ( csv_1char,     "99"); }// integer type
  //TEST( Fcsv, CSV1cstr)     { EXPECT_EQ( csv_1cstr,     "\"c\""); }// string
  TEST( Fcsv, CSVfp12)      {
    EXPECT_EQ( csv_fp12, "1.00000000e+00,2.0000000000000000E+00");
  }
  TEST( Fcsv, CSVff12)      {
    EXPECT_EQ( csv_ff12, "1.00000000e+00,2.00000000e+00");
  }

} }//end femerea::test:: namespace
