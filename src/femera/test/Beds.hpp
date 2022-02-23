#ifndef FEMERA_HAS_BEDS_HPP
#define FEMERA_HAS_BEDS_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Beds;// Derive a CRTP concrete class from Test.
  class Beds : public Test<Beds> { private: friend class Test;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Beds (femera::Work::Core_ptrs_t) noexcept;
    Beds () =delete;//NOTE Use the constructor above.
  };
} }//end femera::test:: namespace

#include "Beds.ipp"

//end FEMERA_BEDS_HPP
#endif
