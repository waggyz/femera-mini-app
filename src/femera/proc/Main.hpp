#ifndef HAS_FEMERA_MAIN_HPP
#define HAS_FEMERA_MAIN_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Main;// Derive a CRTP concrete class from Proc.
  class Main final: public Proc<Main> { private: friend class Proc;
  //TODO friend class Proc<Main> or just Proc? Both work.
  private:
    fmr::Local_int race_n = 0;// number of shared-memory threads
  public:
    static
    fmr::Local_int get_race_ix ();
    fmr::Local_int get_race_n  ();
  public:
    Main (const femera::Work::Core_ptrs_t) noexcept;
    Main () noexcept;
  private:
    fmr::Local_int task_proc_ix () noexcept;
    //fmr::Local_int task_proc_n  ();//TODO
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Main.ipp"

//end HAS_FEMERA_MAIN_HPP
#endif
