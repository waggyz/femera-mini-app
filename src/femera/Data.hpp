#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"

namespace femera {
  
template <typename T>
class Data : public Work {
private:
  using Data_t = std::shared_ptr<T>;
public:
  void          init     (int*, char**) override;
  fmr::Exit_int exit     (fmr::Exit_int err=0) noexcept override;
  Data_t        get_task (size_t i);
  Data_t        get_task (std::vector<size_t> tree);
private:
  T* derived (Data*);
protected:// make it clear this class needs to be inherited
  Data ()            =default;
  Data (const Data&) =default;
  Data (Data&&)      =default;// shallow (pointer) copyable
  Data& operator =
    (const Data&)    =default;
  ~Data ()           =default;
};

}//end femera:: namespace

#include "Data.ipp"

//end FEMERA_TEST_HPP
#endif
