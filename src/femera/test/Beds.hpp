#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#include "../Test.hpp"

namespace femera { namespace test {

class Beds;// ...then derive a CRTP concrete class from Test.
using Beds_t = std::shared_ptr <Beds>;
class Beds : public Test <Beds> {
public:
  Beds (femera::Work::Make_work_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Beds () noexcept;
  void task_exit ();
};

} }//end femera::test:: namespace

#include "Beds.ipp"

//end FEMERA_GTST_HPP
#endif
