#ifndef FEMERA_HAS_TEST_PROC_HPP
#define FEMERA_HAS_TEST_PROC_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Proc;// Derive a CRTP concrete class from Test.
  class Proc : public Test<Proc> { private: friend class Test;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Proc (femera::Work::Core_ptrs) noexcept;
    Proc () =delete;//NOTE Use the constructor above.
  };
} }//end femera::test:: namespace

#include "Proc.ipp"

//end FEMERA_HAS_TEST_PROC_HPP
#endif
