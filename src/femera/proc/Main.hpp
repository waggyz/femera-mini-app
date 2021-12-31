#ifndef HAS_FEMERA_MAIN_HPP
#define HAS_FEMERA_MAIN_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Main;// Derive a CRTP concrete class from Proc.
  class Main : public Proc <Main> {
  public:
    Main (femera::Work::Make_work_t) noexcept;
    Main () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Main.ipp"

//end HAS_FEMERA_MAIN_HPP
#endif
