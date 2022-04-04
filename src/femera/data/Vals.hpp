#ifndef FEMERA_DATA_VALS_HPP
#define FEMERA_DATA_VALS_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Vals;//TODO Derive a CRTP concrete class from Data?
  class Vals : public Data <Vals> { private: friend class Data;
  private:
    proc::Team_t team_id = 0;// MPI communicator
  private:
    Vals (femera::Work::Core_ptrs_t) noexcept;
    Vals () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::data:: namespace

#include "Vals.ipp"

//end FEMERA_DATA_VALS_HPP
#endif