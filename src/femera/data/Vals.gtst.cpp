#include "../femera.hpp"
#include "Vals.hpp"

#include <gtest/gtest.h>

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  auto vals = femera::data::Vals ();
  const auto ints10 = std::string("test-10-ints");
  const auto vals10 = std::string("test-10-floats");
  //
  TEST(Vals, Ints10) {
    EXPECT_EQ( vals.get<int> ("integer name not found"), nullptr);
    EXPECT_EQ( vals.set      (ints10,10,int(1))[9], int(1));
    EXPECT_EQ( vals.get<int> (ints10)     [0], int(1));
    EXPECT_EQ( vals.get<int> (ints10)     [9], int(1));
    EXPECT_EQ( vals.get<int> (ints10,9)   [0], int(1));
  }
  TEST(Vals, Vals10) {
    EXPECT_EQ( vals.get<double> ("floating point name not found"), nullptr);
    EXPECT_EQ( vals.set<double> ("10 zeros",10) [9], double(0.0));
    EXPECT_EQ( vals.set   (vals10,10,1.0) [9], double(1.0));
    EXPECT_EQ( vals.get<double> (vals10)  [0], double(1.0));
    EXPECT_EQ( vals.get<double> (vals10)  [9], double(1.0));
    EXPECT_EQ( vals.get<double> (vals10,9)[0], double(1.0));
    EXPECT_EQ( vals.set<double>
      ("another10", 10, vals.get<double>(vals10))[9], double(1.0));
    EXPECT_EQ( vals.get<double> ("another10")    [0], double(1.0));
  }
  TEST(Vals, Alignment) {
    EXPECT_LE( alignof (double),      FMR_ALIGN_VALS);
    EXPECT_LE( alignof (std::size_t), FMR_ALIGN_INTS);
    EXPECT_GE( alignof (std::size_t), sizeof(fmr::Hash_int));// CRC SIMD calc ok
    EXPECT_EQ( uintptr_t (vals.get<int>    (ints10)) % FMR_ALIGN_INTS, 0);
    EXPECT_EQ( uintptr_t (vals.get<double> (vals10)) % FMR_ALIGN_VALS, 0);
    EXPECT_EQ( uintptr_t (vals.get<double> ("another10")) % FMR_ALIGN_VALS, 0);
  }
  TEST(Vals, AutoConvert) {
    EXPECT_EQ(       vals.get<uint>  (ints10) [0], uint(1));
    EXPECT_FLOAT_EQ( vals.get<float> (vals10) [0], float(1.0));
  }
#if 0
  TEST(Vals, AvxVals) {
    EXPECT_EQ( reinterpret_cast<double*>
      (& vals.set<__m256d> ("avxd10",10)) [0], 0.0);
  }
#endif
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
