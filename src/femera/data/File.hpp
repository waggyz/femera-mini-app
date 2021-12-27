#ifndef FEMERA_FILE_HPP
#define FEMERA_FILE_HPP

#include "../Data.hpp"

namespace femera {

class File;// ...then derive a CRTP concrete class from File.
class File : public Data <File> {
public:
  File (femera::Work::Make_work_t) noexcept;
  File () noexcept;
  void task_exit ();
};

}//end femera:: namespace

#include "File.ipp"

//end FEMERA_FILE_HPP
#endif
