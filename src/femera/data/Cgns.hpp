#ifndef FEMERA_DATA_CGNS_HPP
#define FEMERA_DATA_CGNS_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Cgns;// Derive a CRTP concrete class from Data.
  class Cgns final: public Data <Cgns> { friend class Data;
  private:// typedefs
    enum class Concurrency : fmr::Enum_int {Error=0, Serial,
      Independent, Collective
    };
    enum class File_format : fmr::Enum_int {
      CGNS_none    = 0,// CG_FILE_NONE,// 0
      CGNS_ADF1    = 1,// CG_FILE_ADF ,// 1
      CGNS_HDF5    = 2,// CG_FILE_HDF5,// 2
      CGNS_ADF2    = 3,// CG_FILE_ADF2 // 3
      CGNS_unknown = 4
    };
  private:// member variables
    fmr::Team_int team_id = 0;// MPI communicator
    std::string hdf5_vers = "";
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
    bool does_file (const fmr::Data_name_t&);
  private:
    Cgns (femera::Work::Core_ptrs_t) noexcept;
    Cgns () =delete;//NOTE Use the constructor above.
  };
  //
} }//end femera::data:: namespace

#include "Cgns.ipp"

//end FEMERA_DATA_CGNS_HPP
#endif
