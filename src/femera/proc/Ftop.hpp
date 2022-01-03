#ifndef FEMERA_FTOP_HPP
#define FEMERA_FTOP_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Ftop;// Derive a CRTP concrete class from Proc.
  class Ftop : public Proc <Ftop> {
  public:
    Ftop (femera::Work::Core_t) noexcept;
    Ftop () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Ftop.ipp"

//end FEMERA_FTOP_HPP
#endif
