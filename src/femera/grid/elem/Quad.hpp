#ifndef FEMERA_HAS_GRID_ELEM_QUAD_HPP
#define FEMERA_HAS_GRID_ELEM_QUAD_HPP

#include "Spar.hpp"

namespace femera { namespace grid { namespace elem {

struct Quad : public Elem<Quad> {//TODO Sqr?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 2;
  static constexpr fmr::Local_int vert_n = 4;
  static constexpr fmr::Local_int vols_n = 0;
};

} } }//end femera::grid::elem:: namespace

#include "Quad.ipp"

//end FEMERA_HAS_GRID_ELEM_QUAD_HPP
#endif
