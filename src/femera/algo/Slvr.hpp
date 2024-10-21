#ifndef FEMERA_HAS_ALGO_SLVR_HPP
#define FEMERA_HAS_ALGO_SLVR_HPP

#include "../Algo.hpp"

namespace femera { namespace algo {
  class Slvr;// Derive a CRTP concrete class from Test.
  class Slvr final: public Algo<Slvr> { friend class Algo;// Interface to Slvr class
  private:
//    void scan      (int* argc, char** argv);
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Slvr (femera::Work::Core_ptrs_t) noexcept;
    Slvr () =delete;//NOTE Use the constructor above.
  };
} }//end femera::algo:: namespace

#include "Slvr.ipp"

//end FEMERA_HAS_ALGO_SLVR_HPP
#endif
