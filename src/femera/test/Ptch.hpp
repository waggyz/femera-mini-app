#ifndef FEMERA_HAS_TEST_PTCH_HPP
#define FEMERA_HAS_TEST_PTCH_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Ptch;// Multi-element patch test task
  class Ptch final: public Test<Ptch> { friend class Test;
  //TODO register this as a Data source providing node connectivity & coordinates.
  //     This class only tests geometric aspects of patches:
  //     volume, area, length, element quality, jacs & dets, etc.
  //     It also provides reference data for single, double, and mixed precision
  //     verification.
  private:
    void task_init (int* argc, char** argv);// scan for -T, -P args?
    // Load all patches in mem one per thread.
    // Load a second set with each patch partitioned to run in parallel.
    void task_exit ();
  private:
    Ptch (femera::Work::Core_ptrs_t) noexcept;
    Ptch () =delete;//NOTE Use the constructor above.
  };
} }//end femera::test:: namespace

#include "Ptch.ipp"

//end FEMERA_HAS_TEST_PTCH_HPP
#endif
