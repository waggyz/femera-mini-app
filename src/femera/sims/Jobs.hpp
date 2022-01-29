#ifndef FEMERA_HAS_JOBS_HPP
#define FEMERA_HAS_JOBS_HPP

#include "../Sims.hpp"

namespace femera { namespace sims {
  class Jobs;// Derive a CRTP concrete class from Sims.
  class Jobs : public Sims<Jobs> { private: friend class Sims;
  public:
    Jobs (femera::Work::Core_t) noexcept;
    Jobs () noexcept;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
    // The shared Work::proc, data, test objects are managed by this class.
  };
} }//end femera::sims:: namespace

#include "Jobs.ipp"

//end FEMERA_HAS_JOBS_HPP
#endif
