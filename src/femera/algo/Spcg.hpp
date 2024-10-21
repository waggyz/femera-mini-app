#ifndef FEMERA_HAS_ALGO_SPCG_HPP
#define FEMERA_HAS_ALGO_SPCG_HPP

#include "../Algo.hpp"

namespace femera { namespace algo {
  class Spcg;// Derive a CRTP concrete class from Algo.
  class Spcg final: public Algo<Spcg> { friend class Algo;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Spcg (femera::Work::Core_ptrs_t) noexcept;
    Spcg () =delete;//NOTE Use the constructor above.
  };
} }//end femera::algo:: namespace

#include "Spcg.ipp"

//end FEMERA_HAS_ALGO_SPCG_HPP
#endif
