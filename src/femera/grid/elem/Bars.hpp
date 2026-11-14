#ifndef FEMERA_HAS_GRID_ELEM_BARS_HPP
#define FEMERA_HAS_GRID_ELEM_BARS_HPP

#include "../Elem.hpp"

namespace femera { namespace grid { namespace elem {

//class Bars final: public Elem<Bars> { friend class Elem;
class Bars final : public Elem<Bars> {
  template <class T, fmr::Dim_int P, typename F>
  friend class Elem;
private:// Everything should be private, accessible through the Elem interface.
// Scalars ====================================================================
  static constexpr fmr::math::Poly poly_type = fmr::math::Poly::Serendipity;
  //
  static constexpr fmr::Dim_int   elem_d = 1;
  //
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
  // Arrays & pointers ========================================================
  // vert_conn is trivial: {0,...,get_vert_n-1}
  // elem_conn is trivial: {0,...,get_elem_n()-1}.
  static constexpr fmr::Local_int bars_conn [Bars::vert_n] = {0,1};
  static constexpr fmr::Local_int* tris_conn = nullptr;
  static constexpr fmr::Local_int* quad_conn = nullptr;
  //
  template <typename F=fmr::Geom_float> static constexpr
  F vert_coor [vert_n * 3u]// transposed coor_vert (vert major, dims minor)
    = {           // vertex                      //
    F(0.0), F(0.0), F(0.0),// 0       0-------1           //
    F(1.0), F(0.0), F(0.0) // 1                  o--x     //
  };
  template <typename F=fmr::Geom_float> static constexpr
  F coor_vert [vert_n * 3u]// transposed vert_coor (dims major, vert minor)
    = {
    F(0.0), F(1.0),
    F(0.0), F(0.0),
    F(0.0), F(0.0)
  };
  /* node_coor_p1 same as vert_coor
  fmr::Geom_float node_coor_p2 [(vert_n+1) * sims_d]// transposed coor_node_p2
    = {           // node                        //
    0.0, 0.0, 0.0,// 0       0---2---1           // starts with p1 vert_coor
    0.5, 0.0, 0.0 // 1                  o--x     //
    1.0, 0.0, 0.0 // 2
    edge node 1: x = 0.5*vert0 + 0.5*vert1
    //                  or node 2 = 0.5*vert0 + 0.5*vert1
    // Follow bars_conn for edge node placement.
  };
  fmr::Geom_float node_coor_p3 [(vert_n+2) * sims_d]// transposed coor_node_p3
  #define F13 F(1.0)/F(3)
  #define F23 F(2.0)/F(3)
    = {           // node                        //
    0.0, 0.0, 0.0,// 0      0--3--1--2           //
    F13, 0.0, 0.0 // 1                  o--x     //
    F23, 0.0, 0.0 // 2
    1.0, 0.0, 0.0 // 3
    edge node 1: x = F23*vert0 + F13*vert1
    edge node 2: x = F13*vert0 + F23*vert1
    // Follow bars_conn for edge node placement,
    // then, follow tris_conn and quad_conn for face node placement.
    #undef F13
    #undef F23
  }; */
  // Methods ====================================================================
  //
};//end struct Bars

} } }//end femera::grid::elem:: namespace

#include "Bars.ipp"

//end FEMERA_HAS_GRID_ELEM_BARS_HPP
#endif
