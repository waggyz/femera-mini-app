#include "../femera.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <valarray>
#include <cstring>           //std::memcmp
#include <cstdio>            // std::snprintf

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  
  using Vec alignas (size_t) = std::vector   <int>;
  using Val alignas (size_t) = std::valarray <int>;

  Vec vec8z = {0,0,0,0,0,0,0,0};
  Val val8z = {0,0,0,0,0,0,0,0};
  std::vector   <int> vec8u (8);
  std::valarray <int> val8u (8);

  std::valarray <fmr::Bulk_int> bulk (256);

  inline
  int* vec_resize (size_t sz) {
    vec8u.resize (sz);
    return &vec8u[0];
  }
  inline
  int* val_resize (size_t sz) {
    val8u.resize (sz);
    return &val8u[0];
  }
  inline
  double test_uninit_valarray (const fmr::Local_int sz=100000) {double sum = 0;
    std::vector <double> vec ={};
    vec.reserve (sz);
    // Make valarray from uninitialized reserved vector data.
    std::valarray <double> val (vec.data (), sz);//TODO Does this copy?
    for (fmr::Local_int i =0; i<sz; i++) { sum += val [i]; }
    mini->data->name_line (mini->data->fmrout, "vals from vec1",
      "%g (garbage)", sum);
    return sum;
    /* stackoverflow.com/questions/13634504/assign-a-stdvector-to-a-stdvalarray
    //
    // Make valarray from vector.
    std::valarray <double> val (vec.data (), vec.sz ());
    //
    // Write valarray data into a vector.
    vec.assign (std::begin (val), std::end (val));
    */
  }
  using Vval_t = std::vector<std::valarray<double>>;
  inline
  double sum_uninit_valarray (Vval_t& vals, const fmr::Local_int sz=100000) {
    std::vector <double> vec ={};
    vec.reserve (sz);
    // Make valarray from uninitialized reserved vector data.
    vals.push_back (std::valarray <double> (vec.data (), sz));//TODO copies?
    double sum = 0.0;
    for (fmr::Local_int i =0; i<sz; i++) { sum += vals[0] [i]; }
    mini->data->name_line (mini->data->fmrout, "vals from vec2",
      "%g (garbage)", sum);
    return sum;
  }
  TEST(Mini, IsOK) {
    EXPECT_EQ( 1, 1);
  }
  TEST(ValsInit, VecAndValZeroed) {
    EXPECT_EQ( 0, std::memcmp (&vec8z[0], &val8z[0], sizeof (vec8z)));
  }
  TEST(ValsInit, VecUninitZeroed) {
    EXPECT_EQ( 0, std::memcmp (&vec8u[0], &vec8z[0], sizeof (vec8z)));
  }
  TEST(ValsInit, ValUninitZeroed) {
    EXPECT_EQ( 0, std::memcmp (&val8u[0], &val8z[0], sizeof (val8z)));
  }
  TEST(ValsInit, VecResizeZeroed) {
    EXPECT_EQ( 0, std::memcmp (vec_resize (8192), &vec8z[0], sizeof (vec8z)));
  }
  TEST(ValsInit, ValResizeZeroed) {
    EXPECT_EQ( 0, std::memcmp (val_resize (8192), &val8z[0], sizeof (val8z)));
  }
  TEST(ValsAlign, UnalignedInt) {
    EXPECT_EQ( 4, alignof (int));
  }
  TEST(ValsAlign, AlignedSize_t) {
    EXPECT_EQ( 8, alignof (size_t));
  }
  TEST(ValsAlign, VecUnalignedOK) {
    EXPECT_EQ( 0, reinterpret_cast<size_t>(&vec8u[0]) % alignof (int));
  }
  TEST(ValsAlign, VecAlignedOK) {
    EXPECT_EQ( 0, reinterpret_cast<size_t>(&vec8z[0]) % alignof (size_t));
  }
  TEST(BulkInts, ZeroAsFmrInts) {
    EXPECT_EQ(fmr::Dim_int(0)   ,reinterpret_cast<fmr::Dim_int*>   (&bulk[0])[0]);
    EXPECT_EQ(fmr::Enum_int(0)  ,reinterpret_cast<fmr::Enum_int*>  (&bulk[0])[0]);
    EXPECT_EQ(fmr::Local_int(0) ,reinterpret_cast<fmr::Local_int*> (&bulk[0])[0]);
    EXPECT_EQ(fmr::Global_int(0),reinterpret_cast<fmr::Global_int*>(&bulk[0])[0]);
    EXPECT_EQ(fmr::Perf_int(0)  ,reinterpret_cast<fmr::Perf_int*>  (&bulk[0])[0]);
    EXPECT_EQ(fmr::Exit_int(0)  ,reinterpret_cast<fmr::Exit_int*>  (&bulk[0])[0]);
  }
  TEST(BulkInts, ZeroAsFmrFloats) {
    EXPECT_EQ(fmr::Perf_float(0),reinterpret_cast<fmr::Perf_float*>(&bulk[0])[0]);
    EXPECT_EQ(fmr::Geom_float(0),reinterpret_cast<fmr::Geom_float*>(&bulk[0])[0]);
    EXPECT_EQ(fmr::Phys_float(0),reinterpret_cast<fmr::Phys_float*>(&bulk[0])[0]);
    EXPECT_EQ(fmr::Solv_float(0),reinterpret_cast<fmr::Solv_float*>(&bulk[0])[0]);
    EXPECT_EQ(fmr::Cond_float(0),reinterpret_cast<fmr::Cond_float*>(&bulk[0])[0]);
    EXPECT_EQ(fmr::Post_float(0),reinterpret_cast<fmr::Post_float*>(&bulk[0])[0]);
    EXPECT_EQ(fmr::Plot_float(0),reinterpret_cast<fmr::Plot_float*>(&bulk[0])[0]);
  }
  TEST(BulkInts, ZeroAsStringChars) {
    EXPECT_EQ(char(0), reinterpret_cast<char*> (&bulk[0])[0] );
  }
  TEST(BulkInts, WorkForStrings) {
    EXPECT_EQ( 5, std::snprintf (&bulk[0], bulk.size(),// returns #of chars
      std::string("hello").c_str()) );
    EXPECT_EQ( std::string("hello"), std::string (&bulk[0]) );
  }
  TEST( Valarray, UninitValarray ) {
#if 0
    // This only works sometimes.
    EXPECT_NE( test_uninit_valarray (), 0.0 );//TODO Test performance.
#else
    EXPECT_GT( (test_uninit_valarray () < 0.0) ? 1 : 2, 0 );
    Vval_t vval= {};// vector of valarrays
    EXPECT_EQ( vval.size (), 0 );
    EXPECT_GT( ( sum_uninit_valarray (vval, 100000) < 0.0) ? 1 : 2, 0);
    EXPECT_EQ( vval.size (), 1 );
    EXPECT_EQ( vval[0].size (), 100000 );
    EXPECT_GT( ( vval[0][99999] > 0.0) ? vval[0][99999] : 1, 0);
#endif
  }
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
