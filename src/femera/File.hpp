#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"

namespace femera {
  
template <typename T>
class File : public Work {
private:
  using File_t = std::shared_ptr<T>;
public:
  void          init     (int*, char**) override;
  fmr::Exit_int exit     (fmr::Exit_int err=0) noexcept override;
  File_t        get_task (size_t i);
  File_t        get_task (std::vector<size_t> tree);
private:
  T* derived (File*);
protected:// make it clear this class needs to be inherited
  File ()            =default;
  File (const File&) =default;
  File (File&&)      =default;// shallow (pointer) copyable
  File& operator =
    (const File&)    =default;
  ~File ()           =default;
};

}//end femera:: namespace

#include "File.ipp"

//end FEMERA_TEST_HPP
#endif
