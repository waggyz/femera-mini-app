#include "Beds.hpp"
#include "../proc/Main.hpp"

#ifdef FMR_HAS_GTEST
#include "Gtst.hpp"
#endif
#include "Self.hpp"
#include "Perf.hpp"

#include <unistd.h>          // argc opterr optopt optind optarg
#include <algorithm>

namespace femera {
  void test::Beds::rand_init () {
    if (this->proc == nullptr || !this->proc->did_init ()) {
      FMR_THROW("Random generators require an initialized processing hierarchy");
    }
    int thread_n = 1;
#ifdef FMR_HAS_OPENMP
    if (::omp_get_level () != 0) {
      FMR_THROW("Random generators must be initialized outside OpenMP regions");
    }
    thread_n = ::omp_get_max_threads ();
#endif
    int initialized_n = 1;
    this->rand_states.resize (std::size_t (thread_n));
    FMR_PRAGMA_OMP(omp parallel num_threads(thread_n))
    {
      int thread_i = 0;
#ifdef FMR_HAS_OPENMP
      thread_i = ::omp_get_thread_num ();
      FMR_PRAGMA_OMP(omp master)
      { initialized_n = ::omp_get_num_threads (); }
#endif
      auto& state = this->rand_states[std::size_t (thread_i)];
      state.owner = std::this_thread::get_id ();
      state.proc_id = this->proc->get_proc_id ();
      state.engine.seed (get_rand_seed (state.proc_id));
    }
    this->rand_states.resize (std::size_t (initialized_n));
    std::vector<fmr::Local_int> ids;
    ids.reserve (this->rand_states.size ());
    for (const auto& state : this->rand_states) {
      ids.push_back (state.proc_id);
    }
    std::sort (ids.begin (), ids.end ());
    if (std::adjacent_find (ids.begin (), ids.end ()) != ids.end ()) {
      FMR_THROW("Random generators require distinct processing IDs per thread");
    }
  }
  test::Beds::Rand_int test::Beds::get_rand () {
    if (!this->rand_ready) {
      FMR_THROW("Random generators are not initialized");
    }
    int thread_i = 0;
#ifdef FMR_HAS_OPENMP
    if (::omp_get_level () > 1) {
      FMR_THROW("Random generators do not support nested OpenMP regions");
    }
    thread_i = ::omp_get_thread_num ();
    if (::omp_get_num_threads () > int (this->rand_states.size ())) {
      FMR_THROW("OpenMP team exceeds the initialized random generator count");
    }
#endif
    auto& state = this->rand_states[std::size_t (thread_i)];
    if (state.owner != std::this_thread::get_id () ||
        state.proc_id != this->proc->get_proc_id ()) {
      FMR_THROW("Thread is outside the initialized random generator mapping");
    }
    return state.engine ();
  }
  void test::Beds::task_init (int* argc, char** argv) {
    if (this->rand_ready) {
      FMR_THROW("Test beds are already initialized");
    }
    this->scan (argc, argv);
    this->rand_init ();
#ifdef FMR_HAS_GTEST
    this->add_task (Test<test::Gtst>::new_task (this->get_core()));
#endif
    if (this->do_test) {// -T option in args
      this->add_task (Test<test::Self>::new_task (this->get_core()));
    }
    if (this->do_test || this->do_perf) {// -T or -P option in args
      this->add_task (Test<test::Perf>::new_task (this->get_core()));
    }
    this->rand_ready = true;
    this->set_init (true);
  }
  void test::Beds::scan (int* argc, char** argv) {
    if (argc != nullptr && argv != nullptr) {
      FMR_PRAGMA_OMP(omp MAIN) {//NOTE getopt is NOT thread safe.
        int ac=argc[0];// Copy getopt variables.
        auto oe=opterr; auto oo=optopt; auto oi=optind; auto oa=optarg;
        opterr = 0; int optchar;
        while ((optchar = getopt (argc[0], argv, "TP")) != -1) {
          // T  -T has no argument//NOTE -g gets eaten by MPI
          //NOTE -fmr:T, -fmr:P are working because -f -m -r and -: are skipped,
          //     leaving only the recognized options -T, -P.
          switch (optchar) {
            case 'T':{ this->do_test = true; break; }
            //TODO: this->proc->add_optc (optchar); break; }
            case 'P':{ this->do_perf = true; break; }
        } }
        // Restore getopt variables.
        argc[0]=ac; opterr=oe; optopt=oo; optind=oi; optarg=oa;
  } } }
}//end femera namespace
