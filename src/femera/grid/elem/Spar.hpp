#ifndef FEMERA_HAS_GRID_ELEM_SPAR_HPP
#define FEMERA_HAS_GRID_ELEM_SPAR_HPP

#include "../Elem.hpp"

namespace femera { namespace grid { namespace elem {

struct Spar : public Elem<Spar> {//TODO Bar?
public:
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 1;
  static constexpr fmr::Local_int vert_n = 2;
  static constexpr fmr::Local_int edge_n = 1;
  static constexpr fmr::Local_int tris_n = 0;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 0;
  //
  static constexpr fmr::Geom_float edge_l = 1.0;// total length of edges
  static constexpr fmr::Geom_float face_a = 0.0;// total surface area
  static constexpr fmr::Geom_float elem_v = 0.0;// natural element volume
  //
  static constexpr
  fmr::Local_int vert_conn [vert_n] = {0,1};
  static constexpr
  fmr::Local_int spar_conn [Spar::vert_n * edge_n] = {0,1};
  static constexpr fmr::Local_int* tris_conn = nullptr;
  static constexpr fmr::Local_int* quad_conn = nullptr;
  //
  static constexpr
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {
                  // vertex                      //
    0.0, 0.0, 0.0,// 0       0---1               //
    1.0, 0.0, 0.0 // 1                o--x       //
  };
  static constexpr
  fmr::Geom_float coor_vert [vert_n * sims_d]// transposed vert_coor
    = {
    0.0, 1.0,
    0.0, 0.0,
    0.0, 0.0
  };
};

} } }//end femera::grid::elem:: namespace

#include "Spar.ipp"

//end FEMERA_HAS_GRID_ELEM_SPAR_HPP
#endif
