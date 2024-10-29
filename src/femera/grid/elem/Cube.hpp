#ifndef FEMERA_HAS_GRID_ELEM_CUBE_HPP
#define FEMERA_HAS_GRID_ELEM_CUBE_HPP

#include "Quad.hpp"

namespace femera { namespace grid { namespace elem {

struct Cube : public Elem<Cube> {//TODO Hex?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 3;
  static constexpr fmr::Local_int vert_n = 8;
  static constexpr fmr::Local_int vols_n = 1;
};

} } }//end femera::grid::elem:: namespace

#include "Cube.ipp"

//end FEMERA_HAS_GRID_ELEM_CUBE_HPP
#endif
