#ifndef FEMERA_HAS_TEST_FE1E_HPP
#define FEMERA_HAS_TEST_FE1E_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class FE1e;
  class FE1e final: public Test<FE1e> { friend class Test;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    FE1e (femera::Work::Core_ptrs_t) noexcept;
    FE1e () =delete;
  };
} }//end femera::test:: namespace

#include "FE1e.ipp"

#endif
