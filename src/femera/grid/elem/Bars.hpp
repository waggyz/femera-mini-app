#ifndef FEMERA_HAS_GRID_ELEM_BARS_HPP
#define FEMERA_HAS_GRID_ELEM_BARS_HPP

#include "../Elem.hpp"

namespace femera { namespace grid { namespace elem {

struct Bars : public Elem<Bars> {//TODO Bars?
public:
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 1;
  static constexpr fmr::Local_int vert_n = 2;
  static constexpr fmr::Local_int edge_n = 1;
  static constexpr fmr::Local_int tris_n = 0;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 0;
  //
  static constexpr fmr::Local_int tile_elem_n = 1;
  static constexpr fmr::Local_int tile_node_n = Bars::vert_n;// tiles naturally
  //
  static constexpr fmr::Geom_float edge_l = 1.0;// total length of edges
  static constexpr fmr::Geom_float face_a = 0.0;// total surface area
  static constexpr fmr::Geom_float elem_v = 0.0;// natural element volume
  //
  static constexpr
  fmr::Local_int vert_conn [vert_n] = {0,1};
  static constexpr
  fmr::Local_int bars_conn [Bars::vert_n * Bars::edge_n] = {0,1};
  static constexpr
  fmr::Local_int tile_conn [tile_node_n] = {0,1};
  //
  static constexpr fmr::Local_int* tris_conn = nullptr;
  static constexpr fmr::Local_int* quad_conn = nullptr;
  //
  static constexpr
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {           // vertex                      //
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
  //------------------------- bar shape functions ----------------------------
  //TODO Return poynomial shape function/gradient transposed?
  template <typename F> static inline// single or double
  F* poly_func_2 (F f[2u], const F x[3u]);// Linear 2-node bar
  template <typename F> static inline
  F* poly_grad_2 (F g[2u *3u], const F[]);// x[3u] not used; result is constant
  template <typename F> static inline
  F* poly_grat_2 (F g[2u *3u], const F[]);// transposed gradient
  
  template <typename F> static inline
  F* poly_func_3 (F f[3u], const F x[3u]);// Quadratic 3-node bar
  template <typename F> static inline
  F* poly_grad_3 (F g[3u *3u], const F x[3u]);
  template <typename F> static inline
  F* poly_grat_3 (F g[3u *3u], const F x[3u]);// transposed gradient
  template <typename F> static inline
  F* poly_func_4 (F f[4u], const F x[3u]);// Cubic 4-node bar
  template <typename F> static inline
  F* poly_grad_4 (F g[4u *3u], const F x[3u]);
  template <typename F> static inline
  F* poly_grat_4 (F g[4u *3u], const F x[3u]);// transposed gradient
};//end struct Bars
} } }//end femera::grid::elem:: namespace

#include "Bars.ipp"

//end FEMERA_HAS_GRID_ELEM_BARS_HPP
#endif
