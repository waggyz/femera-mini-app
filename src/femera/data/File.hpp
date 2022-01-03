#ifndef FEMERA_HAS_FILE_HPP
#define FEMERA_HAS_FILE_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class File;// Derive a CRTP concrete class from File.
  class File : public Data <File> {
  public:
    File (femera::Work::Core_t) noexcept;
//    File () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::data namespace

#include "File.ipp"

//end FEMERA_HAS_FILE_HPP
#endif
