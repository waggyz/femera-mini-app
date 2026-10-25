#include "fmr.hpp"
#include "smat.hpp"

#include <gtest/gtest.h>

TEST(FmrSmat, M1x1InverseAndDeterminant) {
  const double mat[] = {2};
  double inv[1];
  double inv_det[2];
  double invt_det[2];

  ASSERT_EQ(0, fmr::math::m1x1_inv_ref(mat, inv));
  EXPECT_DOUBLE_EQ(inv[0], 0.5);
  ASSERT_EQ(0, fmr::math::m1x1_inv_det_ref(mat, inv_det));
  EXPECT_DOUBLE_EQ(inv_det[0], 0.5);
  EXPECT_DOUBLE_EQ(inv_det[1], 2);
  ASSERT_EQ(0, fmr::math::m1x1_invt_det_ref(mat, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[0], 0.5);
  EXPECT_DOUBLE_EQ(invt_det[1], 2);
}

TEST(FmrSmat, M2x2InverseAndDeterminant) {
  const double mat[] = {4, 7, 2, 6};
  const double expected_inv[] = {0.6, -0.7, -0.2, 0.4};
  const double expected_invt[] = {0.6, -0.2, -0.7, 0.4};
  double inv[4];
  double inv_det[5];
  double invt_det[5];

  ASSERT_EQ(0, fmr::math::m2x2_inv_ref(mat, inv));
  for (int i = 0; i < 4; ++i) EXPECT_NEAR(inv[i], expected_inv[i], 1e-12);
  ASSERT_EQ(0, fmr::math::m2x2_inv_a(mat, inv));
  for (int i = 0; i < 4; ++i) EXPECT_NEAR(inv[i], expected_inv[i], 1e-12);
  ASSERT_EQ(0, fmr::math::m2x2_inv_b(mat, inv));
  for (int i = 0; i < 4; ++i) EXPECT_NEAR(inv[i], expected_inv[i], 1e-12);
  ASSERT_EQ(0, fmr::math::m2x2_inv_det_ref(mat, inv_det));
  for (int i = 0; i < 4; ++i) EXPECT_NEAR(inv_det[i], expected_inv[i], 1e-12);
  EXPECT_DOUBLE_EQ(inv_det[4], 10);
  ASSERT_EQ(0, fmr::math::m2x2_invt_det_ref(mat, invt_det));
  for (int i = 0; i < 4; ++i) EXPECT_NEAR(invt_det[i], expected_invt[i], 1e-12);
  EXPECT_DOUBLE_EQ(invt_det[4], 10);
}

TEST(FmrSmat, M3x3InverseAndDeterminant) {
  const double mat[] = {1, 2, 3, 0, 1, 4, 5, 6, 0};
  const double expected_inv[] = {-24, 18, 5, 20, -15, -4, -5, 4, 1};
  const double expected_invt[] = {-24, 20, -5, 18, -15, 4, 5, -4, 1};
  double inv[9];
  double inv_det[10];
  double invt_det[10];

  ASSERT_EQ(0, fmr::math::m3x3_inv_ref(mat, inv));
  for (int i = 0; i < 9; ++i) EXPECT_DOUBLE_EQ(inv[i], expected_inv[i]);
  ASSERT_EQ(0, fmr::math::m3x3_inv_a(mat, inv));
  for (int i = 0; i < 9; ++i) EXPECT_DOUBLE_EQ(inv[i], expected_inv[i]);
  ASSERT_EQ(0, fmr::math::m3x3_inv_det_ref(mat, inv_det));
  for (int i = 0; i < 9; ++i) EXPECT_DOUBLE_EQ(inv_det[i], expected_inv[i]);
  EXPECT_DOUBLE_EQ(inv_det[9], 1);
  ASSERT_EQ(0, fmr::math::m3x3_invt_det_ref(mat, invt_det));
  for (int i = 0; i < 9; ++i) EXPECT_DOUBLE_EQ(invt_det[i], expected_invt[i]);
  EXPECT_DOUBLE_EQ(invt_det[9], 1);
  ASSERT_EQ(0, fmr::math::m3x3_invt_det_a(mat, invt_det));
  for (int i = 0; i < 9; ++i) EXPECT_DOUBLE_EQ(invt_det[i], expected_invt[i]);
  EXPECT_DOUBLE_EQ(invt_det[9], 1);
}

TEST(FmrSmat, M4x4InverseAndDeterminant) {
  const double mat[] = {
    2, 1, 0, 0,
    0, 3, 1, 0,
    0, 0, 4, 1,
    0, 0, 0, 5
  };
  const double expected_inv[] = {
    0.5, -1.0/6, 1.0/24, -1.0/120,
    0, 1.0/3, -1.0/12, 1.0/60,
    0, 0, 0.25, -0.05,
    0, 0, 0, 0.2
  };
  double expected_invt[16];
  double inv[16];
  double inv_det[17];
  double invt_det[17];
  for (int row = 0; row < 4; ++row)
    for (int col = 0; col < 4; ++col)
      expected_invt[row*4 + col] = expected_inv[col*4 + row];

  ASSERT_EQ(0, fmr::math::m4x4_inv_ref(mat, inv));
  for (int i = 0; i < 16; ++i) EXPECT_NEAR(inv[i], expected_inv[i], 1e-12);
  ASSERT_EQ(0, fmr::math::m4x4_inv_det_ref(mat, inv_det));
  for (int i = 0; i < 16; ++i) EXPECT_NEAR(inv_det[i], expected_inv[i], 1e-12);
  EXPECT_DOUBLE_EQ(inv_det[16], 120);
  ASSERT_EQ(0, fmr::math::m4x4_invt_det_ref(mat, invt_det));
  for (int i = 0; i < 16; ++i) EXPECT_NEAR(invt_det[i], expected_invt[i], 1e-12);
  EXPECT_DOUBLE_EQ(invt_det[16], 120);
}

TEST(FmrSmat, NegativeDeterminantsAreReported) {
  const double mat1[] = {-2};
  const double mat2[] = {-2, 0, 0, 3};
  const double mat3[] = {-2, 0, 0, 0, 3, 0, 0, 0, 4};
  const double mat4[] = {
    -2, 0, 0, 0,
    0, 3, 0, 0,
    0, 0, 4, 0,
    0, 0, 0, 5
  };
  double inv[16];
  double inv_det[17];
  double invt_det[17];

  EXPECT_EQ(-1, fmr::math::m1x1_inv_ref(mat1, inv));
  EXPECT_EQ(-1, fmr::math::m1x1_inv_det_ref(mat1, inv_det));
  EXPECT_DOUBLE_EQ(inv_det[1], -2);
  EXPECT_EQ(-1, fmr::math::m1x1_invt_det_ref(mat1, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[1], -2);

  EXPECT_EQ(-1, fmr::math::m2x2_inv_ref(mat2, inv));
  EXPECT_EQ(-1, fmr::math::m2x2_inv_a(mat2, inv));
  EXPECT_EQ(-1, fmr::math::m2x2_inv_b(mat2, inv));
  EXPECT_EQ(-1, fmr::math::m2x2_inv_det_ref(mat2, inv_det));
  EXPECT_DOUBLE_EQ(inv_det[4], -6);
  EXPECT_EQ(-1, fmr::math::m2x2_invt_det_ref(mat2, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[4], -6);

  EXPECT_EQ(-1, fmr::math::m3x3_inv_ref(mat3, inv));
  EXPECT_EQ(-1, fmr::math::m3x3_inv_a(mat3, inv));
  EXPECT_EQ(-1, fmr::math::m3x3_inv_det_ref(mat3, inv_det));
  EXPECT_DOUBLE_EQ(inv_det[9], -24);
  EXPECT_EQ(-1, fmr::math::m3x3_invt_det_ref(mat3, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[9], -24);
  EXPECT_EQ(-1, fmr::math::m3x3_invt_det_a(mat3, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[9], -24);

  EXPECT_EQ(-1, fmr::math::m4x4_inv_ref(mat4, inv));
  EXPECT_EQ(-1, fmr::math::m4x4_inv_det_ref(mat4, inv_det));
  EXPECT_DOUBLE_EQ(inv_det[16], -120);
  EXPECT_EQ(-1, fmr::math::m4x4_invt_det_ref(mat4, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[16], -120);
}

TEST(FmrSmat, SingularMatricesAreNotInvertible) {
  const double mat1[] = {0};
  const double mat2[] = {1, 2, 2, 4};
  const double mat3[] = {1, 2, 3, 2, 4, 6, 0, 1, 1};
  const double mat4[] = {
    1, 2, 3, 4,
    2, 4, 6, 8,
    0, 1, 2, 3,
    4, 3, 2, 1
  };
  double inv[16] = {};
  double inv_det[17] = {};
  double invt_det[17] = {};

  EXPECT_EQ(1, fmr::math::m1x1_inv_ref(mat1, inv));
  EXPECT_EQ(1, fmr::math::m1x1_inv_det_ref(mat1, inv_det));
  EXPECT_DOUBLE_EQ(inv_det[1], 0);
  EXPECT_EQ(1, fmr::math::m1x1_invt_det_ref(mat1, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[1], 0);

  EXPECT_EQ(1, fmr::math::m2x2_inv_ref(mat2, inv));
  EXPECT_EQ(1, fmr::math::m2x2_inv_a(mat2, inv));
  EXPECT_EQ(1, fmr::math::m2x2_inv_b(mat2, inv));
  EXPECT_EQ(1, fmr::math::m2x2_inv_det_ref(mat2, inv_det));
  EXPECT_DOUBLE_EQ(inv_det[4], 0);
  EXPECT_EQ(1, fmr::math::m2x2_invt_det_ref(mat2, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[4], 0);

  EXPECT_EQ(1, fmr::math::m3x3_inv_ref(mat3, inv));
  EXPECT_EQ(1, fmr::math::m3x3_inv_a(mat3, inv));
  EXPECT_EQ(1, fmr::math::m3x3_inv_det_ref(mat3, inv_det));
  EXPECT_DOUBLE_EQ(inv_det[9], 0);
  EXPECT_EQ(1, fmr::math::m3x3_invt_det_ref(mat3, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[9], 0);
  EXPECT_EQ(1, fmr::math::m3x3_invt_det_a(mat3, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[9], 0);

  EXPECT_EQ(1, fmr::math::m4x4_inv_ref(mat4, inv));
  EXPECT_EQ(1, fmr::math::m4x4_inv_det_ref(mat4, inv_det));
  EXPECT_DOUBLE_EQ(inv_det[16], 0);
  EXPECT_EQ(1, fmr::math::m4x4_invt_det_ref(mat4, invt_det));
  EXPECT_DOUBLE_EQ(invt_det[16], 0);
}

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
