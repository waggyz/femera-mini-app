#ifndef FEMERA_HAS_GRID_ELEM_IQUD_HPP
#define FEMERA_HAS_GRID_ELEM_IQUD_HPP

#include "Quad.hpp"

namespace femera { namespace grid { namespace elem {

struct Iqud : public Elem<Iqud> {
  // interface quadrangles, e.g., cohesive elements
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 2;
  static constexpr fmr::Local_int vert_n = 8;
  static constexpr fmr::Local_int edge_n = 8;
  static constexpr fmr::Local_int tris_n = 0;
  static constexpr fmr::Local_int quad_n = 2;
  static constexpr fmr::Local_int vols_n = 0;
  //
  //TODO natural pyramid perimeter, surface area, volume
  static constexpr fmr::Geom_float edge_l // total length of edges
    = 2.0 * Quad::edge_l;
  static constexpr fmr::Geom_float face_a // total surface area
    = 2.0 * Quad::face_a;
  static constexpr fmr::Geom_float elem_v = 0.0;// planar element
  //
  static constexpr
  fmr::Local_int vert_conn [vert_n] = {0,1,2,3, 4,5,6,7};
  static constexpr
  fmr::Local_int spar_conn [Spar::vert_n * edge_n]
    = {
    0,1, 1,2, 2,3, 3,0,
    4,5, 5,6, 6,7, 7,4
  };
  static constexpr fmr::Local_int tris_conn [Quad::vert_n * quad_n]
    = { 
    0,1,2,3, 7,6,5,4
  };
  static constexpr fmr::Local_int* quad_conn = nullptr;
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {
                   // vertex                  |      3---------2 -+           //
    0.0, 0.0, 0.0, // 0                       |     /         /   |           //
    1.0, 0.0, 0.0, // 1                       |    /         /    |   zero    //
    1.0, 1.0, 0.0, // 2      7,3---------2,6  |   /         /     | thickenss //
    0.0, 1.0, 0.0, // 3       /         /     |  0---------1      |           //
                   //        /         /      |                   |           //
    0.0, 0.0, 0.0, // 4     /         /       |      7---------6 -+           //
    1.0, 0.0, 0.0, // 5  4,0---------1,5      |     /         /       z  y    //
    1.0, 1.0, 0.0, // 6                       |    /         /        | /     //
    0.0, 1.0, 0.0  // 7                       |   /         /         |/      //
  };               //                         |  4---------5          o--x    //
  //
  fmr::Geom_float coor_vert [vert_n * sims_d]// transposed vert_coor
    = {
    0.0, 1.0, 1.0, 0.0,   0.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 1.0, 1.0,   0.0, 0.0, 1.0, 1.0,
    0.0, 0.0, 0.0, 0.0,   0.0, 0.0, 0.0, 0.0
  };
};

} } }//end femera::grid::elem:: namespace

#include "Iqud.ipp"

//end FEMERA_HAS_GRID_ELEM_IQUD_HPP
#endif


