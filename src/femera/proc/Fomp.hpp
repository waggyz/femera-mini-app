#ifndef FEMERA_FOMP_HPP
#define FEMERA_FOMP_HPP

#include "../Proc.hpp"
#ifdef FMR_HAS_OPENMP

namespace femera { namespace proc {
  class Fomp;// Derive a CRTP concrete class from Proc.
  class Fomp : public Proc<Fomp> {private: friend class Proc; friend class Main;
  private:
    static bool is_in_parallel () noexcept;
    fmr::Local_int task_proc_ix () noexcept;
  private:
    void scan      (int* argc, char** argv);
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Fomp (femera::Work::Core_ptrs_t) noexcept;
    Fomp () =delete;//NOTE Use the constructor above.
  };
} }//end femera::proc:: namespace

#include "Fomp.ipp"

#endif
//end FEMERA_FOMP_HPP
#endif
