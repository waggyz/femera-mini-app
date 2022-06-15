#ifndef FEMERA_DATA_BANK_HPP
#define FEMERA_DATA_BANK_HPP

#include "../Data.hpp"
#include "Vals.hpp"

namespace femera { namespace data {
  class Bank;// use to synchronize Vals data
  class Bank final: public Data <Bank> { private: friend class Data;//has vtable
    //             because Data has vtable
    // wraps & exposes a vector of Vals (no vtable) instances for fast access
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
