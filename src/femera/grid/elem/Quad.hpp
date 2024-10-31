#ifndef FEMERA_HAS_GRID_ELEM_QUAD_HPP
#define FEMERA_HAS_GRID_ELEM_QUAD_HPP

#include "Spar.hpp"

namespace femera { namespace grid { namespace elem {

struct Quad : public Elem<Quad> {//TODO Sqr?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 2;
  static constexpr fmr::Local_int vert_n = 4;
  static constexpr fmr::Local_int edge_n = 4;
  static constexpr fmr::Local_int tris_n = 0;
  static constexpr fmr::Local_int quad_n = 1;
  static constexpr fmr::Local_int vols_n = 0;
  //
  static constexpr fmr::Geom_float edge_l = 4.0;// total length of edges
  static constexpr fmr::Geom_float face_a = 1.0;// total surface area
  static constexpr fmr::Geom_float elem_v = 0.0;// natural element volume
  //
  static constexpr
  fmr::Local_int vert_conn [vert_n] = {0,1,2,4};
  static constexpr
  fmr::Local_int spar_conn [Spar::vert_n * edge_n] = {0,1, 1,2, 2,3, 3,0};
  static constexpr fmr::Local_int* tris_conn = nullptr;
  static constexpr fmr::Local_int  quad_conn [Quad::vert_n * quad_n] = {0,1,2,3};
  //
  static constexpr
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {
                  // vertex                //
    0.0, 0.0, 0.0,// 0      3----2         //
    1.0, 0.0, 0.0,// 1      |    |    y    //
    1.0, 1.0, 0.0,// 2      |    |    |    //
    0.0, 1.0, 0.0 // 3      0----1    o--x //
  };
  static constexpr
  fmr::Geom_float coor_vert [vert_n * sims_d]// transposed vert_coor
    = {
    0.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 1.0, 1.0,
    0.0, 0.0, 0.0, 0.0
  };
};

} } }//end femera::grid::elem:: namespace

#include "Quad.ipp"

//end FEMERA_HAS_GRID_ELEM_QUAD_HPP
#endif
