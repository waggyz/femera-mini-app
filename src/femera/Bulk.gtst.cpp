#include "femera.hpp"
#include "Bulk.hpp"

#include "gtest/gtest.h"

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
#if 0
  TEST(BulkVals, Offset) {
    EXPECT_EQ( bankals.offset( 8), 0);
    EXPECT_GT( bankals.offset(13), 0);
  }
#endif
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
    EXPECT_LE( sizeof (fmr::Hash_int), FMR_ALIGN_INTS);
    EXPECT_LE( sizeof (fmr::Hash_int), FMR_ALIGN_VALS);
  }
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
