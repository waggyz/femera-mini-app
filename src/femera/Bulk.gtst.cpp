#include "femera.hpp"
#include "Bulk.hpp"

#include "gtest/gtest.h"

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  auto bulk = femera::data::Bulk();
  const auto ints10 = std::string("test-10-ints");
  const auto uninit10 = std::string("test-10-uninit");
  //
  TEST(Bulk, Ints10) {
    EXPECT_EQ( bulk.add (ints10,10,1)[9], 1);
    //EXPECT_NE( bulk.add<int> (uninit10,10)[9], 123);// check is for not segfault
  }
#if 1
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
  TEST(NewVals, Time) {
    EXPECT_EQ( time_make_new (1024*1,1024*16*4), 0);
  }
#endif
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
