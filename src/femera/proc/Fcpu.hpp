#ifndef FEMERA_FCPU_HPP
#define FEMERA_FCPU_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Fcpu;// Derive a CRTP concrete class from Proc.
  class Fcpu : public Proc<Fcpu> {
  public:
    Fcpu (femera::Work::Core_t) noexcept;
    Fcpu () =delete;//NOTE Use the constructor above.
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Fcpu.ipp"

//end FEMERA_FCPU_HPP
#endif
