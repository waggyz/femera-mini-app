#include "femera.hpp"
#include "Bank.hpp"

#include "gtest/gtest.h"

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  auto bank = femera::data::Bank();
  const auto ints10 = std::string("test-10-ints");
  const auto vals10 = std::string("test-10-floats");
  //
  TEST(Bank, Ints10) {
    EXPECT_EQ( bank.get<int> ("integer name not found"), nullptr);
    EXPECT_EQ( bank.set      (ints10,10,int(1))[9], int(1));
    EXPECT_EQ( bank.get<int> (ints10)     [0], int(1));
    EXPECT_EQ( bank.get<int> (ints10)     [9], int(1));
    EXPECT_EQ( bank.get<int> (ints10,9)   [0], int(1));
  }
  TEST(Bank, Vals10) {
    EXPECT_EQ( bank.get<double> ("floating point name not found"), nullptr);
    EXPECT_EQ( bank.set<double> ("10 zeros",10) [9], double(0.0));
    EXPECT_EQ( bank.set   (vals10,10,1.0) [9], double(1.0));
    EXPECT_EQ( bank.get<double> (vals10)  [0], double(1.0));
    EXPECT_EQ( bank.get<double> (vals10)  [9], double(1.0));
    EXPECT_EQ( bank.get<double> (vals10,9)[0], double(1.0));
    EXPECT_EQ( bank.set<double>
      ("another10", 10, bank.get<double>(vals10))[9], double(1.0));
    EXPECT_EQ( bank.get<double> ("another10")    [0], double(1.0));
  }
  TEST(Bank, Alignment) {
    EXPECT_LE( alignof (double),      FMR_ALIGN_VALS);
    EXPECT_LE( alignof (std::size_t), FMR_ALIGN_INTS);
    EXPECT_GE( alignof (std::size_t), sizeof(fmr::Hash_int));// CRC SIMD calc ok
    EXPECT_EQ( uintptr_t (bank.get<int>    (ints10)) % FMR_ALIGN_INTS, 0);
    EXPECT_EQ( uintptr_t (bank.get<double> (vals10)) % FMR_ALIGN_VALS, 0);
    EXPECT_EQ( uintptr_t (bank.get<double> ("another10")) % FMR_ALIGN_VALS, 0);
  }
  TEST(Bank, AutoConvert) {
    EXPECT_EQ(       bank.get<uint>  (ints10) [0], uint(1));
    EXPECT_FLOAT_EQ( bank.get<float> (vals10) [0], float(1.0));
  }
#if 0
  TEST(Bank, AvxVals) {
    EXPECT_EQ( reinterpret_cast<double*>
      (& bank.set<__m256d> ("avxd10",10)) [0], 0.0);
  }
#endif
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
