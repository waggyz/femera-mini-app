#ifndef FEMERA_DATA_BANK_HPP
#define FEMERA_DATA_BANK_HPP

#include "../Data.hpp"
#include "Vals.hpp"

namespace femera { namespace data {
  class Bank;// Derive as a CRTP concrete class from Data.
  class Bank final: public Data <Bank> { private: friend class Data;
    //  has vtable because Data has vtable; use to synchronize Vals data
    // wraps & exposes a vector of Vals (no vtable) instances for fast access
#ifdef FMR_BANK_LOCAL
  private:
    Vals vals = Vals ();
#endif
#ifdef FMR_VALS_LOCAL
  private:
    using Vals_list_t = std::vector<Vals>;
    Vals_list_t vals = {};
  public:
    Vals* vals_ptr (fmr::Local_int thread_ix);
#endif
  public:
    Vals* vals_ptr ();
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
