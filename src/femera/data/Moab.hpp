#ifndef FEMERA_DATA_Moab_HPP
#define FEMERA_DATA_Moab_HPP

#include "../Data.hpp"
//#include "../../fmr/form.hpp"

namespace femera { namespace data {
  class Moab;// Derive a CRTP concrete class from File.
  class Moab : public Data <Moab> { private: friend class Data;
  private:
    Moab (femera::Work::Core_ptrs_t) noexcept;
    Moab () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::data:: namespace

#include "Moab.ipp"

//end FEMERA_DATA_Moab_HPP
#endif