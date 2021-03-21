#include "gtest/gtest.h"
#include "base.h"

#ifdef FMR_DEBUG
#include <cstdio>       // printf
#endif
using namespace Femera;

inline int check_named_type_size (){
  const size_t enum_n = size_t (fmr::enum2val (fmr::Data::end));
  const size_t name_n = fmr::vals_name.size() - 1;
  const size_t info_n = fmr::vals_info.size() - 1;
  if (name_n != enum_n) {
    printf (//fmr::detail::main->proc->log->printf_err (
      "ERROR The size of fmr::vals_name is %lu, but should be %lu.\n",
      name_n, enum_n);
    return 1;
  }
  if (name_n != info_n) {
    printf (//fmr::detail::main->proc->log->printf_err (
      "ERROR The size of fmr::vals_info is %lu, but should be %lu.\n",
      info_n, enum_n);
    return 1;
  }
  return 0;
}
inline int print_datatypes (){
  const fmr::Enum_int enum_n = fmr::enum2val (fmr::Data::end);
  for (fmr::Enum_int i=0; i<enum_n; i++) {
    if (enum_n>0) {
#ifdef FMR_DEBUG
      printf (
#else
      fmr::detail::main->proc->log->printf (//TODO Write to file.
#endif
        "%3u %-16s %s\n",
        i,// fmr::,fmr::Vals_type.c_str(),//TODO Print base type name and size.
        fmr::vals_name[i].c_str(), fmr::vals_info[i].c_str());
  } }
  return enum_n;
}
TEST( Data, SizeofDataGE416 ){
  EXPECT_GE( sizeof(Data), 416 );
}
TEST( Data, CheckNamedDatatypes ) {
  EXPECT_EQ( check_named_type_size(), 0 );
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
