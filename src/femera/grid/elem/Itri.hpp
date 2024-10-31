#ifndef FEMERA_HAS_GRID_ELEM_ITRI_HPP
#define FEMERA_HAS_GRID_ELEM_ITRI_HPP

#include "Tris.hpp"

namespace femera { namespace grid { namespace elem {

struct Itri : public Elem<Itri> {
  // interface triangles, e.g., cohesive elements
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 2;
  static constexpr fmr::Local_int vert_n = 6;
  static constexpr fmr::Local_int edge_n = 6;
  static constexpr fmr::Local_int tris_n = 2;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 0;
  //
  static constexpr fmr::Geom_float edge_l // total length of edges
    = 2.0 * Tris::edge_l;
  static constexpr fmr::Geom_float face_a // total surface area
    = 2.0 * Tris::face_a;
  static constexpr fmr::Geom_float elem_v = 0.0;// planar element
  //
  static constexpr
  fmr::Local_int vert_conn [vert_n] = {0,1,2, 3,4,5};
  static constexpr
  fmr::Local_int spar_conn [Spar::vert_n * edge_n]
    = {
    0,1, 1,2, 2,0,
    3,4, 4,5, 5,3
  };
  static constexpr fmr::Local_int tris_conn [Tris::vert_n * tris_n]
    = { 
    0,1,2, 5,4,3
  };
  static constexpr fmr::Local_int* quad_conn = nullptr;
  static constexpr
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {
                   // vertex             |   5-----4  -+             //
    0.0, 0.0, 0.0, // 0                  |    \   /    |             //
    1.0, 0.0, 0.0, // 1                  |     \ /     |   zero      //
    0.0, 1.0, 0.0, // 2    5,2-----1,4   |      3      | thickness   //
                   //         \   /      |             |             //
    0.0, 0.0, 0.0, // 3        \ /       |   2-----1  -+             //
    1.0, 0.0, 0.0, // 4         0,3      |    \   /        y z x     //
    0.0, 1.0, 0.0  // 5                  |     \ /          \|/      //
  };               //                    |      0            o       //
  //
  static constexpr
  fmr::Geom_float coor_vert [vert_n * sims_d]// transposed vert_coor
    = {
    0.0, 1.0, 0.0,  0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,  0.0, 0.0, 1.0,
    0.0, 0.0, 0.0,  0.0, 0.0, 0.0
  };
};

} } }//end femera::grid::elem:: namespace

#include "Itri.ipp"

//end FEMERA_HAS_GRID_ELEM_ITRI_HPP
#endif

