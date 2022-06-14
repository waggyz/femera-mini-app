#ifndef FEMERA_DATA_BANK_HPP
#define FEMERA_DATA_BANK_HPP

#include "Vals.hpp"

namespace femera { namespace data {
  class Bank;//            Data has vtable
  class Bank final: public Data <Bank> { private: friend class Data;
  public:
    Vals bank = Vals();
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Bank (femera::Work::Core_ptrs_t) noexcept;
    Bank () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data:: namespace

#include "Bank.ipp"

//end FEMERA_DATA_BANK_HPP
#endif
