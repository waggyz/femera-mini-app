#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"

namespace femera {
  
template <typename T>
class Test : public Work {
private:
  using Test_t = std::shared_ptr<T>;
public:
  void          init     (int*, char**) override;
  fmr::Exit_int exit     (fmr::Exit_int err=0) noexcept override;
  Test_t        get_task (size_t i);
  Test_t        get_task (std::vector<size_t> tree);
private:
  T* derived (Test*);
protected:// make it clear this class needs to be inherited
  Test ()            =default;
  Test (const Test&) =default;
  Test (Test&&)      =default;// shallow (pointer) copyable
  Test& operator =
    (const Test&)    =default;
  ~Test ()           =default;
};

}//end femera:: namespace

#include "Test.ipp"

//end FEMERA_TEST_HPP
#endif