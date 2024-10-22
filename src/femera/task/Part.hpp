#ifndef FEMERA_HAS_PART_HPP
#define FEMERA_HAS_PART_HPP

#include "../Task.hpp"

#include "../phys/Mtrl.hpp"
#include "../grid/Cell.hpp"
#include "../algo/Slvr.hpp"

namespace femera { namespace task {
  class Part;// Derive a CRTP concrete class from Task.
  class Part final: public Task<Part> { friend class Task;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    Part (femera::Work::Core_ptrs_t) noexcept;
    Part () =delete;
  };
} }//end femera::task:: namespace

#include "Part.ipp"

//end FEMERA_HAS_PART_HPP
#endif

