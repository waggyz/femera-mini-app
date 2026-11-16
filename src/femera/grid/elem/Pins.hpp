#ifndef FEMERA_HAS_GRID_ELEM_PINS_HPP
#define FEMERA_HAS_GRID_ELEM_PINS_HPP

#include "../Elem.hpp"

namespace femera { namespace grid { namespace elem {

struct Pins : public Elem<Pins> {
  template <class T, fmr::Dim_int P, typename F>
  friend class Elem;
private:
  static constexpr fmr::math::Poly poly_type = fmr::math::Poly::None;
  //
  static constexpr fmr::Dim_int   elem_d = 1;
  static constexpr fmr::Local_int vert_n = 1;
  static constexpr fmr::Local_int edge_n = 0;
  static constexpr fmr::Local_int tris_n = 0, tri3_face_node_n = 0;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 0;
  //
  static constexpr fmr::Geom_float edge_l = 0.0;
  static constexpr fmr::Geom_float face_a = 0.0;
  static constexpr fmr::Geom_float elem_v = 0.0;
  //
  static constexpr fmr::Local_int* bars_vert_conn = nullptr;
  static constexpr fmr::Local_int* tris_vert_conn = nullptr;
  static constexpr fmr::Local_int* quad_vert_conn = nullptr;
  static constexpr
  fmr::Geom_float vert_coor [vert_n * 3u] = {0.0,0.0,0.0};
  //
  static constexpr
  fmr::Geom_float coor_vert [vert_n * 3u] = {0.0,0.0,0.0};
};

} } }//end femera::grid::elem:: namespace

#include "Pins.ipp"

//end FEMERA_HAS_GRID_ELEM_PINS_HPP
#endif
