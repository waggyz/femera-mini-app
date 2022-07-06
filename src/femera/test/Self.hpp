#ifndef FEMERA_HAS_TEST_SELF_HPP
#define FEMERA_HAS_TEST_SELF_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Self;// Derive a CRTP concrete class from Test.
  class Self final: public Test<Self> { private: friend class Test;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
    void log_init_info ();
    fmr::Exit_int proc_node_chck ();
  private:
    Self (femera::Work::Core_ptrs_t) noexcept;
    Self () =delete;//NOTE Use the constructor above.
  };
} }//end femera::test:: namespace

#include "Self.ipp"

//end FEMERA_HAS_TEST_SELF_HPP
#endif
