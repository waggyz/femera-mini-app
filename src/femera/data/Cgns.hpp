#ifndef FEMERA_DATA_Cgns_HPP
#define FEMERA_DATA_Cgns_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Cgns;// Derive a CRTP concrete class from File.
  class Cgns : public Data <Cgns> { private: friend class Data;
  private:
    Cgns (femera::Work::Core_ptrs_t) noexcept;
    Cgns () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::data:: namespace

#include "Cgns.ipp"

//end FEMERA_DATA_Cgns_HPP
#endif
