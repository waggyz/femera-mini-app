#ifndef FEMERA_HAS_BEDS_HPP
#define FEMERA_HAS_BEDS_HPP

#include "../Test.hpp"

#include <limits>
#include <random>
#include <thread>
#include <vector>

namespace femera { namespace test {
  class Beds;// Derive a CRTP concrete class from Test.
  class Beds final: public Test<Beds> { friend class Test;// Interface to Test class
  public:
    using Rand_int = std::mt19937::result_type;
    Rand_int get_rand ();
    static constexpr Rand_int get_rand_max () noexcept;
    static constexpr unsigned int get_rand_seed
      (fmr::Local_int proc_id, unsigned int base_seed=fmr::rand_seed) noexcept;
  private:
    struct Rand_state {
      std::mt19937 engine = {};
      std::thread::id owner = {};
      fmr::Local_int proc_id = 0;
    };
    std::vector<Rand_state> rand_states = {};
    bool rand_ready = false;// Work clears did_init before child-test exit.
    bool do_test = false;
    bool do_perf = false;
  private:
    void scan      (int* argc, char** argv);
    void task_init (int* argc, char** argv);
    void task_exit ();
    void rand_init ();
  private:
    Beds (femera::Work::Core_ptrs_t) noexcept;
    Beds () =delete;//NOTE Use the constructor above.
  };
} }//end femera::test:: namespace

#include "Beds.ipp"

//end FEMERA_BEDS_HPP
#endif
