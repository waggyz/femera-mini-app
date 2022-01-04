#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Gtst;// Derive a CRTP concrete class from Test.
  class Gtst : public Test<Gtst> {
  public:
    Gtst (femera::Work::Core_t) noexcept;
    Gtst () =delete;//NOTE Use the constructor above.
    void task_init (int* argc, char** argv);
    void task_exit ();
};

} }//end femera::test:: namespace

#include "Gtst.ipp"

//end FEMERA_GTST_HPP
#endif
