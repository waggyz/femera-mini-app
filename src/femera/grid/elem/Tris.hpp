#ifndef FEMERA_HAS_GRID_ELEM_TRIS_HPP
#define FEMERA_HAS_GRID_ELEM_TRIS_HPP

#include "Spar.hpp"

#include <cmath>// std::sqrt () needed to calculate edge lengths

namespace femera { namespace grid { namespace elem {

struct Tris : public Elem<Tris> {//TODO Tri?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 2;
  static constexpr fmr::Local_int vert_n = 3;
  static constexpr fmr::Local_int vols_n = 0;
};

} } }//end femera::grid::elem:: namespace

#include "Tris.ipp"

//end FEMERA_HAS_GRID_ELEM_TRIS_HPP
#endif
