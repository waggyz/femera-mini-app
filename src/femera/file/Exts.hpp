#ifndef FEMERA_EXTS_HPP
#define FEMERA_EXTS_HPP

#include "../File.hpp"

namespace femera { namespace file {

class Exts;// ...then derive a CRTP concrete class from File.
using Exts_t = std::shared_ptr <Exts>;
class Exts : public File <Exts> {
public:
  Exts (femera::Work::Make_work_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Exts () noexcept;
  void task_exit ();
};

} }//end femera::file:: namespace

#include "Exts.ipp"

//end FEMERA_EXTS_HPP
#endif
