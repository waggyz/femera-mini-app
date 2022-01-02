#ifndef FEMERA_BEDS_HPP
#define FEMERA_BEDS_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Beds;// Derive a CRTP concrete class from Test.
  class Beds : public Test <Beds> {
  public:
    Beds (femera::Work::Make_work_t) noexcept;
    Beds () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::test:: namespace

#include "Beds.ipp"

//end FEMERA_BEDS_HPP
#endif
