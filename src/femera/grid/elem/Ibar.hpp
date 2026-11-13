#ifndef FEMERA_HAS_GRID_ELEM_IBAR_HPP
#define FEMERA_HAS_GRID_ELEM_IBAR_HPP

#include "Bars.hpp"

namespace femera { namespace grid { namespace elem {

struct Ibar : public Elem<Ibar> {
  // interface bars, e.g., 2D cohesive elements
public:
  static constexpr fmr::Dim_int sims_d = 2;
  static constexpr fmr::Dim_int elem_d = 1;
  static constexpr fmr::Local_int vert_n = 4;
  static constexpr fmr::Local_int edge_n = 2;
  static constexpr fmr::Local_int tris_n = 0;
  static constexpr fmr::Local_int quad_n = 1;
  static constexpr fmr::Local_int vols_n = 0;
  //
  static constexpr fmr::Geom_float edge_l = 2.0;// total length of edges
  static constexpr fmr::Geom_float face_a = 1.0;// total surface area
  static constexpr fmr::Geom_float elem_v = 0.0;// planar element
  //
  //static constexpr
  //fmr::Local_int vert_conn [vert_n] = {0,1, 2,3};
  static constexpr
  fmr::Local_int bars_conn [Elem<Bars>::get_vert_n () * edge_n] = { 0,1, 2,3 };
  static constexpr fmr::Local_int* tris_conn = nullptr;
  static constexpr fmr::Local_int quad_conn [Elem<Bars>::get_vert_n () * edge_n]
    = { 0,1,2,3 };
  static constexpr
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {       // vertex                  |                               //
    0.0, 0.0, // 0                       |      2---------3 -+           //
    1.0, 0.0, // 1                       |                   |   zero    //
    0.0, 0.0, // 2                       |      0---------1 -+           //
    1.0, 0.0  // 3  0,2---------1,3      |                       z  y    //
  };
  static constexpr
  fmr::Geom_float coor_vert [vert_n * sims_d]// transposed vert_coor
    = {
    0.0, 1.0, 0.0, 1.0,
    0.0, 0.0, 0.0, 0.0
  };
};

} } }//end femera::grid::elem:: namespace

#include "Ibar.ipp"

//end FEMERA_HAS_GRID_ELEM_IBAR_HPP
#endif


