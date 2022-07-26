#ifndef FEMERA_DATA_CGNS_HPP
#define FEMERA_DATA_CGNS_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Cgns;// Derive a CRTP concrete class from Data.
  class Cgns final: public Data <Cgns> { private: friend class Data;
  private:
    fmr::Team_int team_id = 0;// MPI communicator
  private:
    Cgns (femera::Work::Core_ptrs_t) noexcept;
    Cgns () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
    bool does_file (const fmr::Data_name_t&){return false;}//TODO to .?pp
  };
} }//end femera::data:: namespace

#include "Cgns.ipp"

//end FEMERA_DATA_CGNS_HPP
#endif
