#ifndef FEMERA_DATA_BANK_HPP
#define FEMERA_DATA_BANK_HPP

#include "Vals.hpp"

namespace femera { namespace data {
  class Bank;// wraps & exposes Vals (no vtable) instance for fast access
  class Bank final: public Data <Bank> { private: friend class Data;//has vtable
    //             becuase Data has vtable
    // use this class to synchronize Vals data
  public:
#ifdef FMR_BANK_LOCAL
    Vals vals = Vals ();
#endif
#ifdef FMR_VALS_LOCAL
    std::vector<Vals> vals = {};
#endif
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
