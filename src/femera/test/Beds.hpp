#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#include "../Test.hpp"

namespace femera {
  class Beds;// Derive a CRTP concrete class from Test.
  class Beds : public Test <Beds> {
  public:
    Beds (femera::Work::Make_work_t) noexcept;
    Beds () noexcept;
    void task_exit ();
  };
}//end femera:: namespace

#include "Beds.ipp"

//end FEMERA_GTST_HPP
#endif
