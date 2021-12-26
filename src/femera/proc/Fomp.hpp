#ifndef FEMERA_FOMP_HPP
#define FEMERA_FOMP_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {

class Fomp;// ...then derive a CRTP concrete class from Proc.
using Fomp_t = std::shared_ptr <Fomp>;
class Fomp : public Proc <Fomp> {
public:
  Fomp (femera::Work::Make_work_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Fomp () noexcept;
  void task_exit ();
};

} }//end femera::proc:: namespace

#include "Fomp.ipp"

//end FEMERA_FOMP_HPP
#endif
