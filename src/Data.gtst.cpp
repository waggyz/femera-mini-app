#include "gtest/gtest.h"
#include "base.h"

#ifdef FMR_DEBUG
#include <cstdio>       // printf
#endif
using namespace Femera;

inline fmr::Enum_int count_named_types (){
  const fmr::Enum_int enum_n = fmr::enum2val (fmr::Data::end);
  const fmr::Enum_int name_n = fmr::Enum_int (fmr::vals_name.size()) - 1;
  const fmr::Enum_int info_n = fmr::Enum_int (fmr::vals_info.size()) - 1;
  if (name_n != enum_n) {
      printf (//fmr::detail::main->proc->log->printf_err (//TODO do this?
        "ERROR size of fmr::vals_name is %u, not %u\n", name_n, enum_n);
      return 0;
  }
  if (name_n != info_n) {
      printf (//fmr::detail::main->proc->log->printf_err (//TODO do this?
        "ERROR size of fmr::vals_info is %u, not %u\n", info_n, enum_n);
      return 0;
  }
  return enum_n;
}
inline int print_datatypes (){
  const fmr::Enum_int enum_n = fmr::enum2val (fmr::Data::end);
  for (fmr::Enum_int i=0; i<enum_n; i++) {
    if (enum_n>0) {
#ifdef FMR_DEBUG
      printf ("%3u %-16s %s\n",
#else
      //TODO Write to file.
      fmr::detail::main->proc->log->printf ("%u %-16s %s\n",
#endif
        i,// fmr::,fmr::Vals_type.c_str(),//TODO Print base type name and size.
        fmr::vals_name[i].c_str(), fmr::vals_info[i].c_str());
  } }
  return enum_n;
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
