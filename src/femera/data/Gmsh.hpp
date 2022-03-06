#ifndef FEMERA_DATA_Gmsh_HPP
#define FEMERA_DATA_Gmsh_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Gmsh;// Derive a CRTP concrete class from File.
  class Gmsh : public Data <Gmsh> { private: friend class Data;
  private:
    using Optval = double;
    using Thrown = const std::basic_string<char>&;
  private:
    bool did_gmsh_init = false;
  private:
    Gmsh (femera::Work::Core_ptrs_t) noexcept;
    Gmsh () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::data:: namespace

#include "Gmsh.ipp"

//end FEMERA_DATA_Gmsh_HPP
#endif
