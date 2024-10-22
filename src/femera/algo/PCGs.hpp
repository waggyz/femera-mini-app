#ifndef FEMERA_HAS_ALGO_SPCG_HPP
#define FEMERA_HAS_ALGO_SPCG_HPP

#include "../Algo.hpp"

namespace femera { namespace algo {
  class PCGs;// Derive a CRTP concrete class from Algo.
  class PCGs final: public Algo<PCGs> { friend class Algo;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    PCGs (femera::Work::Core_ptrs_t) noexcept;
    PCGs () =delete;//NOTE Use the constructor above.
  };
} }//end femera::algo:: namespace

#include "PCGs.ipp"

//end FEMERA_HAS_ALGO_SPCG_HPP
#endif
