#ifndef FEMERA_FOMP_HPP
#define FEMERA_FOMP_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Fomp;// Derive a CRTP concrete class from Proc.
  class Fomp : public Proc <Fomp> {
  public:
    Fomp (femera::Work::Make_work_t) noexcept;
    Fomp () noexcept;
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Fomp.ipp"

//end FEMERA_FOMP_HPP
#endif
