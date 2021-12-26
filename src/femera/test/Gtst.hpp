#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#include "../Test.hpp"

namespace femera { namespace test {

class Gtst;// ...then derive a CRTP concrete class from Test.
using Gtst_t = std::shared_ptr <Gtst>;
class Gtst : public Test <Gtst> {
public:
  Gtst (femera::Work::Make_work_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Gtst () noexcept;
  void task_exit ();
};

} }//end femera::test:: namespace

#include "Gtst.ipp"

//end FEMERA_GTST_HPP
#endif
