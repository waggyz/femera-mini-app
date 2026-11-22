#include "../femera.hpp"

#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <thread>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}
TEST( TestBeds, TaskName ){
  EXPECT_EQ( mini->test->get_abrv (), "beds" );
}

TEST( TestBeds, DrawsDuringChildTestExit ){
  std::mt19937 reference
    (femera::test::Beds::get_rand_seed (mini->proc->get_proc_id ()));
  for (int i = 0; i < 32; ++i) {
    EXPECT_EQ( mini->test->get_rand (), reference () );
  }
}

TEST( TestBeds, SeedModuloWithoutOverflow ){
  using Beds = femera::test::Beds;
  constexpr auto max = std::numeric_limits<unsigned int>::max ();
  EXPECT_EQ( Beds::get_rand_seed (0, 0), 0u );
  EXPECT_EQ( Beds::get_rand_seed (0, max), 0u );
  EXPECT_EQ( Beds::get_rand_seed (1, max - 1), 0u );
  EXPECT_EQ( Beds::get_rand_seed (2, max - 1), 1u );
  EXPECT_EQ( Beds::get_rand_seed (1, max), 1u );
  constexpr auto id_max = std::numeric_limits<fmr::Local_int>::max ();
  EXPECT_EQ( Beds::get_rand_seed (id_max, max),
    std::uintmax_t (id_max) % std::uintmax_t (max) );
  EXPECT_EQ( Beds::get_rand_seed (mini->proc->get_proc_id ()),
    (std::uintmax_t (fmr::rand_seed) + mini->proc->get_proc_id ()) % max );
}

TEST( TestBeds, SerialReferenceAndInstanceIsolation ){
  using Beds = femera::test::Beds;
  const auto first = Beds::new_task (mini->get_core ());
  const auto second = Beds::new_task (mini->get_core ());
  EXPECT_THROW( first->get_rand (), femera::Errs );
  ASSERT_EQ( first->init (nullptr, nullptr), 0 );
  ASSERT_EQ( second->init (nullptr, nullptr), 0 );
  std::mt19937 reference (Beds::get_rand_seed (mini->proc->get_proc_id ()));
  for (int i = 0; i < 32; ++i) {
    const auto expected = reference ();
    EXPECT_EQ( first->get_rand (), expected );
    EXPECT_EQ( second->get_rand (), expected );
  }
  ASSERT_EQ( first->exit (), 0 );
  EXPECT_THROW( first->get_rand (), femera::Errs );
  ASSERT_EQ( first->init (nullptr, nullptr), 0 );
  reference.seed (Beds::get_rand_seed (mini->proc->get_proc_id ()));
  EXPECT_EQ( first->get_rand (), reference () );
  ASSERT_EQ( first->exit (), 0 );
  ASSERT_EQ( second->exit (), 0 );
}

TEST( TestBeds, ThreadSequencesAndSerialParallelContinuity ){
  using Beds = femera::test::Beds;
  const auto beds = Beds::new_task (mini->get_core ());
  ASSERT_EQ( beds->init (nullptr, nullptr), 0 );
  int thread_n = 1;
#ifdef FMR_HAS_OPENMP
  thread_n = ::omp_get_max_threads ();
#endif
  struct Samples {
    fmr::Local_int id = 0;
    std::array<Beds::Rand_int, 32> first = {};
    std::array<Beds::Rand_int, 32> second = {};
  };
  std::vector<Samples> samples (std::size_t (thread_n), Samples {});
  std::mt19937 main_reference
    (Beds::get_rand_seed (mini->proc->get_proc_id ()));
  EXPECT_EQ( beds->get_rand (), main_reference () );
  FMR_PRAGMA_OMP(omp parallel num_threads(1))
  { EXPECT_EQ( beds->get_rand (), main_reference () ); }
  FMR_PRAGMA_OMP(omp parallel)
  {
    int ix = 0;
#ifdef FMR_HAS_OPENMP
    ix = ::omp_get_thread_num ();
    FMR_PRAGMA_OMP(omp master)
    { thread_n = ::omp_get_num_threads (); }
#endif
    auto& s = samples[std::size_t (ix)];
    s.id = mini->proc->get_proc_id ();
    for (auto& value : s.first) { value = beds->get_rand (); }
    // Different draw counts must not affect any other thread's state.
    for (int i = 0; i < ix; ++i) { beds->get_rand (); }
  }
  FMR_PRAGMA_OMP(omp parallel num_threads(thread_n))
  {
    int ix = 0;
#ifdef FMR_HAS_OPENMP
    ix = ::omp_get_thread_num ();
#endif
    for (auto& value : samples[std::size_t (ix)].second) {
      value = beds->get_rand ();
    }
  }
  for (int ix = 0; ix < thread_n; ++ix) {
    const auto& s = samples[std::size_t (ix)];
    std::mt19937 reference (Beds::get_rand_seed (s.id));
    if (ix == 0) { reference.discard (2); }
    for (const auto value : s.first) { EXPECT_EQ( value, reference () ); }
    for (int i = 0; i < ix; ++i) { reference (); }
    for (const auto value : s.second) { EXPECT_EQ( value, reference () ); }
    for (int j = 0; j < ix; ++j) {
      EXPECT_NE( s.id, samples[std::size_t (j)].id );
    }
  }
  main_reference.discard (64);
  EXPECT_EQ( beds->get_rand (), main_reference () );
  ASSERT_EQ( beds->exit (), 0 );
}

TEST( TestBeds, RejectsForeignAndNestedThreads ){
  const auto beds = femera::test::Beds::new_task (mini->get_core ());
  ASSERT_EQ( beds->init (nullptr, nullptr), 0 );
  bool rejected = false;
  std::thread foreign ([&] {
    try { beds->get_rand (); }
    catch (const femera::Errs&) { rejected = true; }
  });
  foreign.join ();
  EXPECT_TRUE( rejected );
#ifdef FMR_HAS_OPENMP
  FMR_PRAGMA_OMP(omp parallel num_threads(1))
  {
    FMR_PRAGMA_OMP(omp parallel num_threads(1))
    { EXPECT_THROW( beds->get_rand (), femera::Errs ); }
  }
#endif
  ASSERT_EQ( beds->exit (), 0 );
}

TEST( TestBeds, RejectsUninitializedProcessingHierarchy ){
  const auto beds = femera::test::Beds::new_task
    (femera::Work::Core_ptrs_t {nullptr, nullptr, nullptr});
  EXPECT_EQ( beds->init (nullptr, nullptr), 1 );
  EXPECT_THROW( beds->get_rand (), femera::Errs );
}

#ifdef FMR_HAS_OPENMP
TEST( TestBeds, RejectsLargerTeamsAndParallelInitialization ){
  const int original_n = ::omp_get_max_threads ();
  const int original_dynamic = ::omp_get_dynamic ();
  ::omp_set_dynamic (0);
  ::omp_set_num_threads (1);
  const auto beds = femera::test::Beds::new_task (mini->get_core ());
  const auto uninitialized = femera::test::Beds::new_task (mini->get_core ());
  const auto err = beds->init (nullptr, nullptr);
  ::omp_set_num_threads (original_n);
  EXPECT_EQ( err, 0 );
  FMR_PRAGMA_OMP(omp parallel num_threads(2))
  { EXPECT_THROW( beds->get_rand (), femera::Errs ); }
  FMR_PRAGMA_OMP(omp parallel num_threads(1))
  { EXPECT_EQ( uninitialized->init (nullptr, nullptr), 1 ); }
  ::omp_set_dynamic (original_dynamic);
  EXPECT_THROW( uninitialized->get_rand (), femera::Errs );
  ASSERT_EQ( beds->exit (), 0 );
}
#endif

TEST( TestBeds, RepeatableBarsPerturbations ){
  using Beds = femera::test::Beds;
  const auto first = Beds::new_task (mini->get_core ());
  const auto second = Beds::new_task (mini->get_core ());
  ASSERT_EQ( first->init (nullptr, nullptr), 0 );
  ASSERT_EQ( second->init (nullptr, nullptr), 0 );
  constexpr std::size_t elem_n = 9;
  constexpr double spacing = 1.0 / elem_n;
  std::array<double, elem_n + 1> coor = {};
  for (std::size_t n = 0; n <= elem_n; ++n) { coor[n] = double (n) * spacing; }
  for (std::size_t n = 1; n < elem_n; ++n) {
    const double unit = double (first->get_rand ()) / Beds::get_rand_max ();
    const double repeat = double (second->get_rand ()) / Beds::get_rand_max ();
    EXPECT_GE( unit, 0.0 );
    EXPECT_LE( unit, 1.0 );
    EXPECT_EQ( unit, repeat );
    const double perturbation = 0.99 * spacing * (unit - 0.5);
    EXPECT_LE( std::abs (perturbation), 0.495 * spacing );
    coor[n] += perturbation;
  }
  EXPECT_EQ( coor.front (), 0.0 );
  EXPECT_EQ( coor.back (), 1.0 );
  for (std::size_t n = 0; n < elem_n; ++n) {
    EXPECT_GT( coor[n + 1] - coor[n], 0.0 );
  }
  ASSERT_EQ( first->exit (), 0 );
  ASSERT_EQ( second->exit (), 0 );
}
