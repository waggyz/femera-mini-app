#ifndef FEMERA_HAS_JOBS_HPP
#define FEMERA_HAS_JOBS_HPP

#include "../Task.hpp"

namespace femera { namespace sims {
  class Jobs;// Derive a CRTP concrete class from Task.
  class Jobs : public Task<Jobs> { private: friend class Task;
  // The shared Work::proc, data, test objects are managed by this class.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    Jobs (femera::Work::Core_ptrs_t) noexcept;
    Jobs () noexcept;
  };
} }//end femera::sims:: namespace

#include "Jobs.ipp"

//end FEMERA_HAS_JOBS_HPP
#endif
