#ifndef FEMERA_DATA_GMSH_HPP
#define FEMERA_DATA_GMSH_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Gmsh;// Derive a CRTP concrete class from Data.
  class Gmsh : public Data <Gmsh> { private: friend class Data;
  private:
    using Number = double;// for ::gmsh::option::get/setNumber (..)
    using Thrown = const std::basic_string<char>&;//TODO std::string& ?
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

//end FEMERA_DATA_GMSH_HPP
#endif