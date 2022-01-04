#ifndef FEMERA_FOMP_HPP
#define FEMERA_FOMP_HPP

#ifdef FMR_HAS_OMP
#include "../Proc.hpp"

namespace femera { namespace proc {
  class Fomp;// Derive a CRTP concrete class from Proc.
  class Fomp : public Proc<Fomp> {
  public:
    Fomp (femera::Work::Core_t) noexcept;
    Fomp () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Fomp.ipp"

#endif
//end FEMERA_FOMP_HPP
#endif
