#ifndef FEMERA_DATA_CGNS_HPP
#define FEMERA_DATA_CGNS_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Cgns;// Derive a CRTP concrete class from Data.
  class Cgns final: public Data <Cgns> { private: friend class Data;
  private:
    proc::Team_t team_id = 0;// MPI communicator
  private:
    Cgns (femera::Work::Core_ptrs_t) noexcept;
    Cgns () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::data:: namespace

#include "Cgns.ipp"

//end FEMERA_DATA_CGNS_HPP
#endif
