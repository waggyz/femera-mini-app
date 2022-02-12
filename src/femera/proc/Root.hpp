#ifndef FEMERA_FTOP_HPP
#define FEMERA_FTOP_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Root;// Derive a CRTP concrete class from Proc.
  class Root : public Proc<Root> { friend class Proc;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
    fmr::Local_int task_proc_ix ();
  private:
    Root (femera::Work::Core_ptrs) noexcept;
    Root () =delete;//NOTE Use the constructor above.
  };
} }//end femera::proc:: namespace

#include "Root.ipp"

//end FEMERA_FTOP_HPP
#endif
