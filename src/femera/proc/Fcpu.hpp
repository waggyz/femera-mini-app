#ifndef FEMERA_FCPU_HPP
#define FEMERA_FCPU_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {

class Fcpu;// ...then derive a CRTP concrete class from Proc.
class Fcpu : public Proc <Fcpu> {
public:
  Fcpu (femera::Work::Make_work_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Fcpu () noexcept;
  void task_exit ();
};

} }//end femera::proc:: namespace

#include "Fcpu.ipp"

//end FEMERA_FCPU_HPP
#endif
