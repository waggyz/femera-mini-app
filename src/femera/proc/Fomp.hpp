#ifndef FEMERA_FOMP_HPP
#define FEMERA_FOMP_HPP

#ifdef FMR_HAS_OMP
#include "../Proc.hpp"

namespace femera { namespace proc {
  class Fomp;// Derive a CRTP concrete class from Proc.
  class Fomp : public Proc<Fomp> {//private: friend class Proc;
  private:
    static bool is_in_parallel ();
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
    fmr::Local_int task_proc_ix ();
  public:
    Fomp (femera::Work::Core_ptrs) noexcept;
    Fomp () =delete;//NOTE Use the constructor above.
  };
} }//end femera::proc:: namespace

#include "Fomp.ipp"

#endif
//end FEMERA_FOMP_HPP
#endif
