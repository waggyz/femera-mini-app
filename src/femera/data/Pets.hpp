#ifndef FEMERA_DATA_PETS_HPP
#define FEMERA_DATA_PETS_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Pets;// Derive a CRTP concrete class from Data.
  class Pets : public Data <Pets> { private: friend class Data;
  private:
    proc::Team_t team_id = 0;// MPI communicator
  private:
    Pets (femera::Work::Core_ptrs_t) noexcept;
    Pets () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::data:: namespace

#include "Pets.ipp"

//end FEMERA_DATA_PETS_HPP
#endif
