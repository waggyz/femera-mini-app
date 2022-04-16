#ifndef FEMERA_HAS_RUNS_HPP
#define FEMERA_HAS_RUNS_HPP

#include "../Task.hpp"

namespace femera { namespace task {
  class Runs;// Derive a CRTP concrete class from Task.
  class Runs final: public Task<Runs> { private: friend class Task;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    Runs (femera::Work::Core_ptrs_t) noexcept;
    Runs () =delete;
  };
} }//end femera::task:: namespace

#include "Runs.ipp"

//end FEMERA_HAS_RUNS_HPP
#endif
