#ifndef FEMERA_HAS_SIMS_HPP
#define FEMERA_HAS_SIMS_HPP

#include "../Task.hpp"

#include "../phys/Mtrl.hpp"
#include "../grid/Cell.hpp"
#include "../algo/Slvr.hpp"

namespace femera { namespace task {
  class Sims;// Derive a CRTP concrete class from Task.
  class Sims final: public Task<Sims> { friend class Task;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    Sims (femera::Work::Core_ptrs_t) noexcept;
    Sims () =delete;
  };
} }//end femera::task:: namespace

#include "Sims.ipp"

//end FEMERA_HAS_SIMS_HPP
#endif

