#ifndef FEMERA_HAS_GRID_ELEM_TETS_HPP
#define FEMERA_HAS_GRID_ELEM_TETS_HPP

#include "Spar.hpp"
#include "Tris.hpp"

namespace femera { namespace grid { namespace elem {

struct Tets : public Elem<Tets> {//TODO Tet?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 3;
  static constexpr fmr::Local_int vert_n = 4;
  static constexpr fmr::Local_int edge_n = 6;
  static constexpr fmr::Local_int tris_n = 4;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 1;
  static constexpr fmr::Local_int conn_n = 4;
  //
  static constexpr fmr::Geom_float edge_l  // total length of edges
    = 3.0 + 3.0*(std::sqrt(2.0));
  static constexpr fmr::Geom_float face_a // total surface area
    = 1.5 + std::sqrt(2.0) * std::sqrt(1.5);
  static constexpr fmr::Geom_float elem_v  // natural element volume
    = 1.0 / 6.0;
  //
  //NOTE Gmsh element conventions
  static constexpr
    fmr::Local_int vert_conn [vert_n] = {0,1,2,3};
  static constexpr
  fmr::Local_int spar_conn [Spar::vert_n * edge_n]
    = {
    0,1, 1,2, 2,0, 0,3, 2,3, 1,3
  };
  static constexpr
  fmr::Local_int tris_conn [Tris::vert_n * tris_n]
    = { //NOTE the normals point inward. This might be wrong.
    0,1,2, 0,3,1, 0,2,3, 1,3,2
  };
  static constexpr
  fmr::Local_int* quad_conn = nullptr;
};

} } }//end femera::grid::elem:: namespace

#include "Tets.ipp"

//end FEMERA_HAS_GRID_ELEM_TETS_HPP
#endif
