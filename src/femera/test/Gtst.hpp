#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  //
  class Gtst;// Derive a CRTP concrete class from Test.
  class Gtst : public Test <Gtst> {
  public:
    Gtst (femera::Work::Make_work_t) noexcept;
    Gtst () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
};

} }//end femera::test:: namespace

#include "Gtst.ipp"

//end FEMERA_GTST_HPP
#endif
