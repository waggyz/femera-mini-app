#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Fmpi;// Derive a CRTP concrete class from Proc.
  class Fmpi : public Proc <Fmpi> {
  public:
    Fmpi (femera::Work::Make_work_t) noexcept;
    Fmpi () noexcept;
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Fmpi.ipp"

//end FEMERA_GTST_HPP
#endif
