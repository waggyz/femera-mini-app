#ifndef FEMERA_HAS_FILE_HPP
#define FEMERA_HAS_FILE_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class File;// Derive a CRTP concrete class from File.
  class File : public Data <File> { private: friend class Data;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    File (femera::Work::Core_ptrs) noexcept;
    File () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data namespace

#include "File.ipp"

//end FEMERA_HAS_FILE_HPP
#endif
