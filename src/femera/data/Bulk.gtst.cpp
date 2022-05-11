#include "../femera.hpp"
#include "Bulk.hpp"

#include <gtest/gtest.h>

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  auto bulk_vals = femera::data::Bulk<FMR_ALIGN_VALS> ();
  //
  TEST(Bulk, IntSizes) {
    EXPECT_EQ( sizeof (fmr::Bulk_int), 1);
    EXPECT_LE( sizeof (fmr::Bulk_int), sizeof (fmr::Dim_int));
    EXPECT_LE( sizeof (fmr::Bulk_int), sizeof (fmr::Enum_int));
    EXPECT_LE( sizeof (fmr::Bulk_int), sizeof (fmr::Local_int));
    EXPECT_LE( sizeof (fmr::Bulk_int), sizeof (fmr::Global_int));
    EXPECT_LE( sizeof (fmr::Bulk_int), sizeof (fmr::Perf_int));
  }
  TEST(Bulk, FloatSizes) {
    EXPECT_LE( sizeof (fmr::Bulk_int), sizeof (float));
    EXPECT_LE( sizeof (fmr::Bulk_int), sizeof (double));
    EXPECT_LE( sizeof (fmr::Bulk_int), sizeof (long double));
    EXPECT_LE( sizeof (float),         FMR_ALIGN_VALS);
    EXPECT_LE( sizeof (double),        FMR_ALIGN_VALS);
#if 0
    EXPECT_LE( sizeof (long double)  , FMR_ALIGN_VALS);
#endif
  }
  TEST(Bulk, CrcHashSizes) {// bulk data padded to align size
    EXPECT_LE( sizeof (fmr::Hash_int), FMR_ALIGN_VALS);
    EXPECT_LE( sizeof (fmr::Hash_int), FMR_ALIGN_INTS);
    EXPECT_LE( 8, FMR_ALIGN_VALS);// for _mm_crc32_u64 (..)
    EXPECT_LE( 8, FMR_ALIGN_INTS);
  }
  TEST(Bulk, CrcHash) {
    EXPECT_EQ( bulk_vals.make_hash<uint32_t> (), 0);
    EXPECT_EQ( bulk_vals.make_hash<uint64_t> (), 0);
    EXPECT_EQ( bulk_vals.make_hash<fmr::Hash_int> (), 0);
    EXPECT_EQ( bulk_vals.set (99, 1.0)[98], 1.0);
    EXPECT_NE( bulk_vals.make_hash<fmr::Hash_int> (), 0);
    EXPECT_NE( bulk_vals.make_hash<uint32_t> (), 0);
    EXPECT_NE( bulk_vals.make_hash<uint64_t> (), 0);
    EXPECT_NE( bulk_vals.make_hash<uint32_t> (),
      uint64_t(bulk_vals.make_hash<uint64_t> ()));
  }
  //
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
