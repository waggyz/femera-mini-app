#ifndef FEMERA_HAS_JOBS_HPP
#define FEMERA_HAS_JOBS_HPP

#include "../Task.hpp"

namespace femera { namespace task {
  class Jobs;// Derive a CRTP concrete class from Task.
  class Jobs final: public Task<Jobs> { friend class Task;//Task classes intrfc
  /* This is the base Femera  class. 
   * Use a Jobs instance to add Runs, Sims, and Part instances to its task list.
   * The shared Work::proc, data, test objects are managed by this class.
   */
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
//    Sims add_sims ();
  public:
    Jobs (femera::Work::Core_ptrs_t) noexcept;
    Jobs () noexcept;
  };
} }//end femera::task:: namespace

#include "Jobs.ipp"

//end FEMERA_HAS_JOBS_HPP
#endif
