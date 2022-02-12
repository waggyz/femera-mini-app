#ifndef FEMERA_NVID_HPP
#define FEMERA_NVID_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Nvid;// Derive a CRTP concrete class from Proc.
  class Nvid : public Proc<Nvid> { friend class Proc;
  private:
    static bool is_in_parallel ();
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
    fmr::Local_int task_proc_ix ();
  private:
    Nvid (femera::Work::Core_ptrs) noexcept;
    Nvid () =delete;//NOTE Use the constructor above.
  };
} }//end femera::proc:: namespace

#include "Nvid.ipp"

//end FEMERA_NVID_HPP
#endif
