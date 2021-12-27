#ifndef FEMERA_MAIN_HPP
#define FEMERA_MAIN_HPP

#include "../Proc.hpp"

namespace femera {

class Main;// ...then derive a CRTP concrete class from Proc.
class Main : public Proc <Main> {
public:
  Main (femera::Work::Make_work_t) noexcept;
  Main () noexcept;
  void task_exit ();
};

}//end femera:: namespace

#include "Main.ipp"

//end FEMERA_MAIN_HPP
#endif
