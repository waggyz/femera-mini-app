#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#ifdef FMR_HAS_MPI
#include "../Proc.hpp"

namespace femera { namespace proc {
  class Fmpi;// Derive a CRTP concrete class from Proc.
  class Fmpi : public Proc<Fmpi> {
  public:
    Fmpi (femera::Work::Core_t) noexcept;
    Fmpi () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Fmpi.ipp"

#endif
//end FEMERA_GTST_HPP
#endif
