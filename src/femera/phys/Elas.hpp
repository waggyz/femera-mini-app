#ifndef FEMERA_HAS_PHYS_ELAS_HPP
#define FEMERA_HAS_PHYS_ELAS_HPP

#include "../Phys.hpp"

namespace femera { namespace phys {
  class Elas;// Derive a CRTP concrete class from Phys.
  class Elas final: public Phys<Elas> { friend class Phys;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Elas (femera::Work::Core_ptrs_t) noexcept;
    Elas () =delete;//NOTE Use the constructor above.
  };
} }//end femera::phys:: namespace

#include "Elas.ipp"

//end FEMERA_HAS_PHYS_ELAS_HPP
#endif
