#ifndef FEMERA_HAS_TEST_PERF_HPP
#define FEMERA_HAS_TEST_PERF_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Perf;// Derive a CRTP concrete class from Test.
  class Perf : public Test<Perf> { private: friend class Test;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Perf (femera::Work::Core_ptrs) noexcept;
    Perf () =delete;//NOTE Use the constructor above.
  };
} }//end femera::test:: namespace

#include "Perf.ipp"

//end FEMERA_HAS_TEST_PERF_HPP
#endif
