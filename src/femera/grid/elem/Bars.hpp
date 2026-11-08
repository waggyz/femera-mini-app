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
  //static constexpr fmr::Local_int tile_node_n = Bars::get_node_n();// tiles naturall
  static constexpr fmr::Local_int tile_vert_n = Bars::vert_n;// tiles naturally have vert_n vertices
  //
  static constexpr fmr::Geom_float edge_l = 1.0;// total length of edges
  static constexpr fmr::Geom_float face_a = 0.0;// total surface area
  static constexpr fmr::Geom_float elem_v = 0.0;// natural element volume
  //
  static constexpr fmr::math::Poly poly_type = fmr::math::Poly::Full;
  //
  static constexpr
  fmr::Local_int vert_conn [Bars::vert_n] = {0,1};
  static constexpr
  fmr::Local_int bars_conn [Bars::vert_n] = {0,1};
  //fmr::Local_int bars_conn [Bars::vert_n + (P-1) * Bars::edge_n] = {0,1};
  static constexpr
  fmr::Local_int tile_conn [2] = {0,1};
  //fmr::Local_int tile_conn [tile_node_n] = {0,1};//FIXME handle p2, p3
  //
  static constexpr fmr::Local_int* tris_conn = nullptr;
  static constexpr fmr::Local_int* quad_conn = nullptr;
  //
  static constexpr
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {           // vertex                      //
    0.0, 0.0, 0.0,// 0       0-------1           //
    1.0, 0.0, 0.0 // 1                o--x       //
  };
  static constexpr
  fmr::Geom_float coor_vert [vert_n * sims_d]// transposed vert_coor
    = {
    0.0, 1.0,
    0.0, 0.0,
    0.0, 0.0
  };
  /* node_coor_p1 same as vert_coor//TODO FIXME move this to Elem.ipp
  fmr::Geom_float node_coor_p2 [(vert_n+1) * sims_d]// transposed coor_node_p2
    = {           // node                        //
    0.0, 0.0, 0.0,// 0       0---2---1           // starts with p1 vert_coor
    1.0, 0.0, 0.0 // 1                o--x       //
    0.5, 0.0, 0.0 // 2: edge node = (vert0 + vert1) * 0.5
    //                  or node 2 = 0.5*vert0 + 0.5*vert1
    // Follow bars_conn for edge node placement.
  };
  fmr::Geom_float node_coor_p3 [(vert_n+2) * sims_d]// transposed coor_node_p3
  #define F13 F(1.0)/F(3)
  #define F23 F(2.0)/F(3)
    = {           // node                        //
    0.0, 0.0, 0.0,// 0      0--2--3--1           //
    1.0, 0.0, 0.0 // 1                o--x       //
    F13, 0.0, 0.0 // 2: edge node = F13*vert0 + F23*vert1)
    F23, 0.0, 0.0 // 3: edge node = F23*vert0 + F13*vert1)
    // Follow bars_conn for edge node placement,
    // Then, follow tris_conn and quad_conn for face node placement.
    #undef F13
    #undef F23
  };
  */
  //------------------------- bar shape functions ----------------------------
  //TODO Return poynomial shape function/gradient transposed?
  template <typename F> static inline// single or double
  F* poly_func_2 (F f[2u], const F x[3u]);// Linear 2-node bar
  template <typename F> static inline
  F* poly_grad_2 (F g[2u *3u], const F[]);// x[3u] not used; result is constant
  template <typename F> static inline
  F* poly_grat_2 (F g[2u *3u], const F[]);// transposed gradient
  //
  template <typename F> static inline
  F* poly_func_3 (F f[3u], const F x[3u]);// Quadratic 3-node bar
  template <typename F> static inline
  F* poly_grad_3 (F g[3u *3u], const F x[3u]);
  template <typename F> static inline
  F* poly_grat_3 (F g[3u *3u], const F x[3u]);// transposed gradient
  //
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
