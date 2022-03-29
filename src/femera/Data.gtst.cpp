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
  Vec vec8u (8);
  Val val8u (8);

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
  TEST(Vals, VecAndValZeroed) {
    EXPECT_EQ( 0, std::memcmp (&vec8z[0], &val8z[0], sizeof (vec8z)));
  }
  TEST(Vals, VecUninitZeroed) {
    EXPECT_EQ( 0, std::memcmp (&vec8u[0], &vec8z[0], sizeof (vec8z)));
  }
  TEST(Vals, ValUninitZeroed) {
    EXPECT_EQ( 0, std::memcmp (&val8u[0], &val8z[0], sizeof (val8z)));
  }
  TEST(Vals, VecResizeZeroed) {
    EXPECT_EQ( 0, std::memcmp (vec_resize (8192), &vec8z[0], sizeof (vec8z)));
  }
  TEST(Vals, ValResizeZeroed) {
    EXPECT_EQ( 0, std::memcmp (val_resize (8192), &val8z[0], sizeof (val8z)));
  }
} }//end femerea::test:: namespace
