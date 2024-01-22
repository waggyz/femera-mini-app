#ifndef FEMERA_GTST_HPP
#define FEMERA_GTST_HPP

#include "../Test.hpp"

namespace femera { namespace test {
  class Gtst;// Derive a CRTP concrete class from Test.
  class Gtst final: public Test<Gtst> { friend class Test;
  private:
    bool is_enabled           = false;
    bool did_run_all_tests    = false;
    bool do_all_tests_on_exit = true ;
  private:
    bool do_enable (int* argc, char** argv);
    void scan      (int* argc, char** argv);
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Gtst (femera::Work::Core_ptrs_t) noexcept;
    Gtst () =delete;//NOTE Use the constructor above.
  };
} }//end femera::test:: namespace

#include "Gtst.ipp"

//end FEMERA_GTST_HPP
#endif
