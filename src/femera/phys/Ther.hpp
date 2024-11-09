#ifndef FEMERA_HAS_PHYS_THER_HPP
#define FEMERA_HAS_PHYS_THER_HPP

#include "../Phys.hpp"

namespace femera { namespace phys {
  class Ther;// Derive a CRTP concrete class from Phys.
  class Ther final: public Phys<Ther> { friend class Phys;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Ther (femera::Work::Core_ptrs_t) noexcept;
    Ther () =delete;//NOTE Use the constructor above.
  };
} }//end femera::phys:: namespace

#include "Ther.ipp"

//end FEMERA_HAS_PHYS_THER_HPP
#endif
