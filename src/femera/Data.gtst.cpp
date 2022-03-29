#include "Data.hpp"

#include <vector>
#include <valarray>
#include <cstring>           //std::memcmp

#include "gtest/gtest.h"

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
namespace femera { namespace test {
  using Vec alignas (size_t) = std::vector   <int>;
  using Val alignas (size_t) = std::valarray <int>;

  Vec vec8z = {0,0,0,0,0,0,0,0};
  Val val8z = {0,0,0,0,0,0,0,0};
  std::vector   <int> vec8u (8);
  std::valarray <int> val8u (8);

  inline
  int* vec_resize (size_t sz) {
    vec8u.resize(sz);
    return &vec8u[0];
  }
  inline
  int* val_resize (size_t sz) {
    val8u.resize(sz);
    return &val8u[0];
  }
  TEST(EarlyData, IsOK) {
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
  TEST(ValsAlign, VecUnalignedOK) {
    EXPECT_EQ( 0, reinterpret_cast<size_t>(&vec8u[0]) % alignof (int));
  }
  TEST(ValsAlign, AlignedSize_t) {
    EXPECT_EQ( 8, alignof (size_t));
  }
  TEST(ValsAlign, VecAlignedOK) {
    EXPECT_EQ( 0, reinterpret_cast<size_t>(&vec8z[0]) % alignof (size_t));
  }
} }//end femerea::test:: namespace
