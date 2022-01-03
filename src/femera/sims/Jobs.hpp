#ifndef FEMERA_HAS_JOBS_HPP
#define FEMERA_HAS_JOBS_HPP

#include "../Sims.hpp"

namespace femera { namespace sims {
  class Jobs;// Derive a CRTP concrete class from Sims.
  class Jobs : public Sims <Jobs> {
  public:
    Jobs (femera::Work::Core_t) noexcept;
    Jobs () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Main_t proc_uniq = nullptr;// processing hierarchy (proc::Main_t)
    File_t data_uniq = nullptr;// data and file handling (data::File)
    Beds_t test_uniq = nullptr;// correctness and performance testing {test::Beds}
  };
} }//end femera::sims:: namespace

#include "Jobs.ipp"

//end FEMERA_HAS_JOBS_HPP
#endif
