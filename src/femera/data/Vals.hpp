#ifndef FEMERA_DATA_VALS_HPP
#define FEMERA_DATA_VALS_HPP

#include "../Bulk.hpp"

namespace femera { namespace data {
  class Vals;//            Data has vtable
  class Vals final: public Data <Vals> { private: friend class Data;
  public:
    Bulk bulk = Bulk();
    proc::Team_t team_id = 0;// MPI communicator
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Vals (femera::Work::Core_ptrs_t) noexcept;
    Vals () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data:: namespace

#include "Vals.ipp"

//end FEMERA_DATA_VALS_HPP
#endif
