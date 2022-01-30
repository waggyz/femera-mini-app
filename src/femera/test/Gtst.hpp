#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Gtst;// Derive a CRTP concrete class from Test.
  class Gtst : public Test<Gtst> { private: friend class Test;
  private:
    bool is_enabled = false;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Gtst (femera::Work::Core_ptrs) noexcept;
    Gtst () =delete;//NOTE Use the constructor above.
};
} }//end femera::test:: namespace

#include "Gtst.ipp"

//end FEMERA_GTST_HPP
#endif
