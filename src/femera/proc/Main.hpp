#ifndef FEMERA_MAIN_HPP
#define FEMERA_MAIN_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {

class Main;// ...then derive a CRTP concrete class from Proc.
class Main : public Proc <Main> {
public:
  Main (femera::Work::Make_work_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Main () noexcept;
  void task_exit ();
};

} }//end femera::proc:: namespace

#include "Main.ipp"

//end FEMERA_MAIN_HPP
#endif
