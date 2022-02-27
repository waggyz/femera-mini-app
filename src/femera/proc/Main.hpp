#ifndef HAS_FEMERA_MAIN_HPP
#define HAS_FEMERA_MAIN_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Main;// Derive a CRTP concrete class from Proc.
  class Main : public Proc<Main> { private: friend class Proc;
  //TODO friend class Proc<Main> or just Proc? Both work.
  public:
    Main (const femera::Work::Core_ptrs_t) noexcept;
    Main () noexcept;
  private:
    fmr::Local_int task_proc_ix ();
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Main.ipp"

//end HAS_FEMERA_MAIN_HPP
#endif
