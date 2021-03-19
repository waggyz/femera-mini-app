#include "gtest/gtest.h"
#include "base.h"

#ifdef FMR_DEBUG
#include <cstdio>       // printf
#endif
using namespace Femera;

inline int count_named_types (){
  const fmr::Enum_int enum_n = fmr::enum2val (fmr::Data::end);
  int name_n = 0;
  for (fmr::Enum_int i=0; i<enum_n; i++) {
    const fmr::Data dt = fmr::Data (i);
    name_n += (fmr::Data_name.count (dt)>0) ? 1 : 0;
  }
  return name_n;
}
inline int print_datatypes (){
  const fmr::Enum_int enum_n = fmr::enum2val (fmr::Data::end);
  int name_n = 0;
  for (fmr::Enum_int i=0; i<enum_n; i++) {
    const fmr::Data dt = fmr::Data (i);
    if (fmr::Data_name.count (dt)>0) {
      const auto npair = fmr::Data_name.at (dt);
      name_n++;
#ifdef FMR_DEBUG
      printf ("%u %-16s %s\n", i, npair.first.c_str(), npair.second.c_str());
#else
      //TODO Write to file.
      fmr::detail::main->proc->log->printf ("%u %-16s %s\n",
        i, npair.first.c_str(), npair.second.c_str());
#endif
  } }
  return name_n;
}
TEST( Data, SizeofDataGE416 ){
  EXPECT_GE( sizeof(Data), 416 );
}
TEST( Data, CountNamedDatatypes ) {
  EXPECT_GT( count_named_types(), 5 );
}
TEST( Data, PrintDatatypes ) {
  EXPECT_GT( print_datatypes(), 5 );
}
#if 0
TEST( Data, TestHas6Sims ){
  EXPECT_EQ( fmr::get_sims_n(), 6 );
}
#endif
#if 0
TEST( Data, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
#endif
#if 0
TEST( DataDummy, ReturnsNonzero ){
  EXPECT_NE(  0     , data_inline_dumm());
#endif
int main(int argc, char** argv ){
  // gtest run_all_tests is done during fmr::exit(err).
  return fmr::exit (fmr::init (&argc,argv) );// These are being tested
}
