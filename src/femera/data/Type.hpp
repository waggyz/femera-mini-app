#ifndef FEMERA_TYPE_HPP
#define FEMERA_TYPE_HPP

#include "../Data.hpp"

namespace femera { namespace data {

class Type;// ...then derive a CRTP concrete class from Data.
using Type_t = std::shared_ptr <Type>;
class Type : public Data <Type> {
public:
  Type (femera::Work::Make_work_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Type () noexcept;
  void task_exit ();
};

} }//end femera::data:: namespace

#include "Type.ipp"

//end FEMERA_TYPE_HPP
#endif
