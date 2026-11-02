#ifndef FEMERA_HAS_PART_HPP
#define FEMERA_HAS_PART_HPP

//#include "../Task.hpp"

//#include "../phys/Mtrl.hpp"
//#include "../grid/Cell.hpp"
//#include "../algo/Slvr.hpp"

//#include "../algo/BCs.hpp"// needed?
//#include "../data/Post.hpp"// needed?

#include "Runs.hpp"

namespace femera { namespace task {
  class Part;// Derive a CRTP concrete class from Task.
  class Part final: public Task<Part> { friend class Task;
    // A Part represents a region of space which may be discretized into
    // a structured Grid (FDms, FVms, FEms) or unstructured Mesh (FEms)
    // with a solver Algo (algorithm),
    // and has associated material properties, boundary conditions,
    // and solver settings.
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

