#ifndef FEMERA_HAS_TEST_PTCH_HPP
#define FEMERA_HAS_TEST_PTCH_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Ptch;// Multi-element patch test task; cases consume shared Mesh fixtures.
  class Ptch final: public Test<Ptch> { friend class Test;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Ptch (femera::Work::Core_ptrs_t) noexcept;
    Ptch () =delete;//NOTE Use the constructor above.
  };
} }//end femera::test:: namespace

#include "Ptch.ipp"

//end FEMERA_HAS_TEST_PTCH_HPP
#endif
