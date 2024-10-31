#ifndef FEMERA_HAS_GRID_ELEM_TRIS_HPP
#define FEMERA_HAS_GRID_ELEM_TRIS_HPP

#include "Spar.hpp"

namespace femera { namespace grid { namespace elem {

struct Tris : public Elem<Tris> {//TODO Tri?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 2;
  static constexpr fmr::Local_int vert_n = 3;
  static constexpr fmr::Local_int edge_n = 3;
  static constexpr fmr::Local_int tris_n = 1;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 0;
  //
  static constexpr fmr::Geom_float edge_l       // total length of edges
    = 2.0 + std::sqrt (2.0);
  static constexpr fmr::Geom_float face_a = 0.5;// total surface area
  static constexpr fmr::Geom_float elem_v = 0.0;// natural element volume
  //
  static constexpr
  fmr::Local_int vert_conn [vert_n] = {0,1,2};
  static constexpr
  fmr::Local_int spar_conn [Spar::vert_n * edge_n] = {0,1, 1,2, 2,0};
  static constexpr fmr::Local_int  tris_conn [Tris::vert_n * tris_n] = {0,1,2};
  static constexpr fmr::Local_int* quad_conn = nullptr;
  //
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {
                  // vertex  2              //
    0.0, 0.0, 0.0,// 0       |\      y      //
    1.0, 0.0, 0.0,// 1       | \     |      //
    0.0, 1.0, 0.0 // 2       0--1    o--x   //
  };
  fmr::Geom_float coor_vert [vert_n * sims_d]// transposed vert_coor
    = {
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    0.0, 0.0, 0.0
  };
};

} } }//end femera::grid::elem:: namespace

#include "Tris.ipp"

//end FEMERA_HAS_GRID_ELEM_TRIS_HPP
#endif
