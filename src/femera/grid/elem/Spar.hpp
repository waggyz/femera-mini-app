#ifndef FEMERA_HAS_GRID_ELEM_SPAR_HPP
#define FEMERA_HAS_GRID_ELEM_SPAR_HPP

#include "../Elem.hpp"

namespace femera { namespace grid { namespace elem {

struct Spar : public Elem<Spar> {//TODO Bar?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 1;
  static constexpr fmr::Local_int vert_n = 2;
  static constexpr fmr::Local_int vols_n = 0;
};

} } }//end femera::grid::elem:: namespace

#include "Spar.ipp"

//end FEMERA_HAS_GRID_ELEM_SPAR_HPP
#endif
