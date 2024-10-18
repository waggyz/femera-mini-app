#ifndef FEMERA_HAS_MTRL_HPP
#define FEMERA_HAS_MTRL_HPP

#include "../Phys.hpp"

namespace femera { namespace phys {
  class Mtrl;// Derive a CRTP concrete class from Test.
  class Mtrl final: public Phys<Mtrl> { friend class Phys;// Interface to Test class
  private:
    bool do_test = false;
    bool do_perf = false;
  private:
//    void scan      (int* argc, char** argv);
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Beds (femera::Work::Core_ptrs_t) noexcept;
    Beds () =delete;//NOTE Use the constructor above.
  };
} }//end femera::phys:: namespace

#include "Mtrl.ipp"

//end FEMERA_HAS_MTRL_HPP
#endif
