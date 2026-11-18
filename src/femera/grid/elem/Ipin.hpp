#ifndef FEMERA_HAS_GRID_ELEM_IPIN_HPP
#define FEMERA_HAS_GRID_ELEM_IPIN_HPP

#include "../Elem.hpp"

namespace femera { namespace grid { namespace elem {

struct Ipin : public Elem<Ipin> {
  template <class T, fmr::Dim_int P, typename F>
  friend class Elem;
private:
  static constexpr fmr::math::Poly poly_type = fmr::math::Poly::None;
  //
  static constexpr fmr::Dim_int   elem_d = 1;
  static constexpr fmr::Local_int vert_n = 2;
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

  template <typename F> static constexpr
  F vert_coor [vert_n * 3u] = {// vert major, dims minor (transposed coor_vert)
    F(0.0), F(0.0), F(0.0),
    F(0.0), F(0.0), F(0.0) 
  };
  template <typename F> static constexpr
  F coor_vert [vert_n * 3u] = {// dims major, vert minor (transposed vert_coor)
    F(0.0), F(0.0),
    F(0.0), F(0.0),
    F(0.0), F(0.0)
  };
};

} } }//end femera::grid::elem:: namespace

#include "Ipin.ipp"

//end FEMERA_HAS_GRID_ELEM_IPIN_HPP
#endif
