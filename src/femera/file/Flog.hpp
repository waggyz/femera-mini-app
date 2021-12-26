#ifndef FEMERA_FLOG_HPP
#define FEMERA_FLOG_HPP

#include "../File.hpp"

namespace femera { namespace file {

class Flog;// ...then derive a CRTP concrete class from File.
using Flog_t = std::shared_ptr <Flog>;
class Flog : public File <Flog> {
public:
  Flog (femera::Work::Make_work_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Flog () noexcept;
  void task_exit ();
};

} }//end femera::file:: namespace

#include "Flog.ipp"

//end FEMERA_FLOG_HPP
#endif
