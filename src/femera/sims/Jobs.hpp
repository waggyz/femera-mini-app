#ifndef FEMERA_HAS_JOBS_HPP
#define FEMERA_HAS_JOBS_HPP

#include "../Sims.hpp"

namespace femera { namespace sims {
  class Jobs;// Derive a CRTP concrete class from Sims.
  class Jobs : public Sims <Jobs> {
  public:
    Jobs (femera::Work::Make_work_t) noexcept;
    Jobs () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::sims:: namespace

#include "Jobs.ipp"

//end FEMERA_HAS_JOBS_HPP
#endif
