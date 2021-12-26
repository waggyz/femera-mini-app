#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {

class Fmpi;// ...then derive a CRTP concrete class from Proc.
using Fmpi_t = std::shared_ptr <Fmpi>;
class Fmpi : public Proc <Fmpi> {
public:
  Fmpi (femera::Work::Make_work_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Fmpi () noexcept;
  void task_exit ();
};

} }//end femera::proc:: namespace

#include "Fmpi.ipp"

//end FEMERA_GTST_HPP
#endif
