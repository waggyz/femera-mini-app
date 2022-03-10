#ifndef FEMERA_HAS_SIMS_HPP
#define FEMERA_HAS_SIMS_HPP

#include "../Task.hpp"

namespace femera { namespace task {
  class Sims;// Derive a CRTP concrete class from Task.
  class Sims : public Task<Sims> { private: friend class Task;
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
