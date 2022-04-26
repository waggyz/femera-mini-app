#include "femera.hpp"
#include "Bulk.hpp"

#include "gtest/gtest.h"

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  auto bulkv = femera::data::Bulk();
  const auto ints10 = std::string("test-10-ints");
  const auto vals10 = std::string("test-10-floats");
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
    EXPECT_LE( sizeof (float)        , FMR_ALIGN_VALS);
    EXPECT_LE( sizeof (double)       , FMR_ALIGN_VALS);
#if 0
    EXPECT_LE( sizeof (long double)  , FMR_ALIGN_VALS);
#endif
  }
  TEST(Bulk, CrcHashSizes) {// bulk data padded to align size
    EXPECT_LE( sizeof (fmr::Hash_int), FMR_ALIGN_INTS);
    EXPECT_LE( sizeof (fmr::Hash_int), FMR_ALIGN_VALS);
  }
  TEST(Bulk, Ints10) {
    EXPECT_EQ( bulkv.add      (ints10,10,1)[9], int(1));
    EXPECT_EQ( bulkv.get<int> (ints10)     [9], int(1));
    EXPECT_EQ( bulkv.get<int> (ints10,9)   [0], int(1));
    EXPECT_EQ( bulkv.get<int> ("not here"), nullptr);
  }
  TEST(Bulk, Vals10) {
    EXPECT_EQ( bulkv.add   (vals10,10,1.0) [9], double(1.0));
    EXPECT_EQ( bulkv.get<double> (vals10)  [9], double(1.0));
    EXPECT_EQ( bulkv.get<double> (vals10,9)[0], double(1.0));
    EXPECT_EQ( bulkv.add<double>
      ("another10", 10, bulkv.get<double>(vals10))[9], double(1.0));
    EXPECT_EQ( bulkv.get<double> ("another10")  [0], double(1.0));
  }
  TEST(Bulk, Alignment) {// bulk data padded to align size
    EXPECT_EQ( uintptr_t (bulkv.get<int>    (ints10)) % FMR_ALIGN_INTS, 0);
    EXPECT_EQ( uintptr_t (bulkv.get<double> (vals10)) % FMR_ALIGN_VALS, 0);
    EXPECT_EQ( uintptr_t (bulkv.get<double> ("another10")) % FMR_ALIGN_VALS, 0);
  }
#if 0
  inline
  int time_make_new (uint n=1024*1, uint sz=1024*1) {
    if (n<1 || sz<1) { return 1; }
    auto time = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>();
    auto vecs = std::vector<std::vector<double>>(n);
    {// Allocate program memory.
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(sz,1);
      }
      double sum=0;
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        for (uint j=0; j<sz; j++) {
        sum += vecs[i].data()[j];
      } }
      printf ("  using: %lu bytes\n", size_t(sum) * sizeof(double));
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(0);
      }
    }
    {
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>();
        vecs[i].reserve (sz);
        vecs[i].data ()[0] = 1.0;// first-touch allocate
      }
      const auto make_time = time.add_busy_time_now ();
      double sum=0;
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        for (uint j=0; j<sz; j++) {
        sum += vecs[i].data()[j];
      } }
      const auto sum_time = time.add_busy_time_now ();
      printf ("reserve: %.2e s, sum %.2e s, total %.2e s, sum %9.0f, size %lu\n",
        double(make_time), double(sum_time), double(make_time+sum_time),
        double(sum), vecs[0].size());
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(0);
      }
    }
    { auto tmp = std::vector<double>();
      tmp.reserve (sz);
      tmp.data () [0] = 1.0;// first-touch allocate
      const auto ptr = tmp.data ();
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>();
        vecs[i].reserve (sz);
        vecs[i].data ()[0] = 1.0;// first-touch allocate
        vecs[i].assign (ptr, ptr + sz);
      }
      const auto make_time = time.add_busy_time_now ();
      double sum=0;
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        for (uint j=0; j<sz; j++) {
        sum += vecs[i].data()[j];
      } }
      const auto sum_time = time.add_busy_time_now ();
      printf (" assign: %.2e s, sum %.2e s, total %.2e s, sum %9.0f, size %lu\n",
        double(make_time), double(sum_time), double(make_time+sum_time),
        double(sum), vecs[0].size());
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(0);
      }
    }
    {
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>();
        vecs[i].reserve (sz);
        auto ptr = vecs[i].data ();
//        ptr [0] = 1.0;// first-touch allocate
        for (size_t j=0; j<sz; j++) { vecs[i].push_back (ptr[j]); }
      }
      const auto make_time = time.add_busy_time_now ();
      double sum=0;
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        for (uint j=0; j<sz; j++) {
        sum += vecs[i].data()[j];
      } }
      const auto sum_time = time.add_busy_time_now ();
      printf ("   push: %.2e s, sum %.2e s, total %.2e s, sum %9.0f, size %lu\n",
        double(make_time), double(sum_time), double(make_time+sum_time),
        double(sum), vecs[0].size());
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(0);
      }
    }
    {
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>();
        vecs[i].reserve (sz);
        vecs[i].data ()[0] = 1.0;// first-touch allocate
      }
      const auto make_time = time.add_busy_time_now ();
      double sum=0;
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        const auto ptr = vecs[i].data ();
        for (uint j=0; j<sz; j++) {
          vecs[i].push_back (ptr[j]);
          sum += vecs[i].data()[j];
      } }
      const auto sum_time = time.add_busy_time_now ();
      printf ("sumpush: %.2e s, sum %.2e s, total %.2e s, sum %9.0f, size %lu\n",
        double(make_time), double(sum_time), double(make_time+sum_time),
        double(sum), vecs[0].size());
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(0);
      }
    }
    {
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(sz);
      }
      const auto make_time = time.add_busy_time_now ();
      double sum=0;
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        for (uint j=0; j<sz; j++) {
        sum += vecs[i].data()[j];
      } }
      const auto sum_time = time.add_busy_time_now ();
      printf ("default: %.2e s, sum %.2e s, total %.2e s, sum %9.0f, size %lu\n",
        double(make_time), double(sum_time), double(make_time+sum_time),
        double(sum), vecs[0].size());
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(0);
      }
    }
    {
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(sz,1.0);
      }
      const auto make_time = time.add_busy_time_now ();
      double sum=0;
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        for (uint j=0; j<sz; j++) {
        sum += vecs[i].data()[j];
      } }
      const auto sum_time = time.add_busy_time_now ();
      printf ("init 1.: %.2e s, sum %.2e s, total %.2e s, sum %9.0f, size %lu\n",
        double(make_time), double(sum_time), double(make_time+sum_time),
        double(sum), vecs[0].size());
      for (uint i=0; i<n; i++) {
        vecs[i] = std::vector<double>(0);
      }
    }
    return 0;
  }
  inline
  int time_bulk (uint N=10, uint n=1024*1, uint sz=1024*1) {// returns total kB
    if (N<1 || n<1 || sz<1) { return 1; }
    auto time = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>();
    auto vals = femera::data::Bulk();
    const double bytes = double (n * sz * sizeof (double));
    for (uint v=0; v<N; v++ ) {
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        vals.add ("vals_"+std::to_string (i), sz, double (v % 2));
      }
      const auto secs = double (time.add_busy_time_now ());
      for (uint i=0; i<n; i++) {
        vals.del<double> ("vals_"+std::to_string (i));
      }
      printf ("%.2e B / %.2e s = %.2e B/s of double (%u)\n",
        bytes, secs, bytes / secs, v % 2);
    }
    return int (N * bytes / 1024.0);
  }
  TEST(NewVals, Time) {
    EXPECT_EQ( time_make_new (1024*1, 1024*16*4), 0);
    EXPECT_EQ( time_bulk (10, 1024*1, 1024*16*4),
      10 * 1024/1024 * 1024*16*4 * sizeof(double));
  }
#endif
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
