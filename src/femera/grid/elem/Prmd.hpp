#ifndef FEMERA_HAS_GRID_ELEM_PRMD_HPP
#define FEMERA_HAS_GRID_ELEM_PRMD_HPP

#include "Tris.hpp"
#include "Quad.hpp"

namespace femera { namespace grid { namespace elem {

struct Prmd : public Elem<Prmd> {//TODO Sqr?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 3;
  static constexpr fmr::Local_int vert_n = 5;
  static constexpr fmr::Local_int vols_n = 1;
};

} } }//end femera::grid::elem:: namespace

#include "Prmd.ipp"

//end FEMERA_HAS_GRID_ELEM_PRMD_HPP
#endif
