#ifndef FEMERA_HAS_GRID_ELEM_BARS_HPP
#define FEMERA_HAS_GRID_ELEM_BARS_HPP

#include "../Elem.hpp"

namespace femera { namespace grid { namespace elem {

//class Bars final: public Elem<Bars> { friend class Elem;
class Bars final : public Elem<Bars> {
  template <class T, fmr::Local_int P, typename F>
  friend class Elem;
// Scalars ====================================================================
public:
  static constexpr fmr::Local_int vert_n = 2;// needed by other elements?
private:
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 1;
  //
  static constexpr fmr::Local_int edge_n = 1;
  static constexpr fmr::Local_int tris_n = 0;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 0;
  //
  static constexpr fmr::Local_int tile_elem_n = 1;// # this type element / tile
  //
  // Bars tile naturally, so tile counts match element counts.
  static constexpr fmr::Local_int tile_vert_n = Bars::vert_n;
  static constexpr fmr::Local_int tile_edge_n = Bars::edge_n;
  static constexpr fmr::Local_int tile_tris_n = Bars::tris_n;
  static constexpr fmr::Local_int tile_quad_n = Bars::quad_n;
  static constexpr fmr::Local_int tile_vols_n = Bars::vols_n;
  //
  static constexpr fmr::Geom_float edge_l = 1.0;// total length of edges
  static constexpr fmr::Geom_float face_a = 0.0;// total surface area
  static constexpr fmr::Geom_float elem_v = 0.0;// natural element volume
  //
  static constexpr fmr::math::Poly poly_type = fmr::math::Poly::Full;
//
// Arrays & pointers ==========================================================
  //static constexpr
  //fmr::Local_int vert_conn [Bars::vert_n] = {0,1};// always integers [0,vert_n-1]
  // elem_conn is an array [0,get_conn_n()-1].
  //TODO these are vertex connectivity.
  static constexpr fmr::Local_int bars_conn [Bars::vert_n] = {0,1};
  static constexpr fmr::Local_int* tris_conn = nullptr;
  static constexpr fmr::Local_int* quad_conn = nullptr;
//
// Methods ====================================================================
  static constexpr
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {           // vertex                      //
    0.0, 0.0, 0.0,// 0       0-------1           //
    1.0, 0.0, 0.0 // 1                  o--x     //
  };
  static constexpr
  fmr::Geom_float coor_vert [vert_n * sims_d]// transposed vert_coor
    = {
    0.0, 1.0,
    0.0, 0.0,
    0.0, 0.0
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
  public://TODO make private, accessed through Elem.
  // Bar shape functions and gradients ========================================
  // Includes transposed poynomial shape function/gradient
  // These bridge between Geom (input) and Phys (output) classes.
  //TODO FIXME these can be dispactched on the polynomial order P (1,2,3),
  //           and adapt to arguments of different precision.
  //NOTE the size of f is get_poly_func_size(1) = node_n for order P
  //     the size of g is get_poly_grad_size(1) = node_n * sims_d for order P
  //     x is sized for 3D, even if not all are dimensions used.
  template <typename Y, typename G> static inline
  Y* calc_poly_func_p1 (Y f[2u], const G x[3u]);// Linear 2-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static inline
  Y* calc_poly_grad_p1 (Y g[2u *3u], const G[]);// x[] not used; result is constant
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static inline
  Y* calc_poly_grat_p1 (Y g[2u *3u], const G[]);// transposed gradient
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static inline
  Y* calc_poly_func_p2 (Y f[3u], const G x[3u]);// Quadratic 3-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static inline
  Y* calc_poly_grad_p2 (Y g[3u *3u], const G x[3u]);
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static inline
  Y* calc_poly_grat_p2 (Y g[3u *3u], const G x[3u]);// transposed gradient
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static inline
  Y* calc_poly_func_p3 (Y f[4u], const G x[3u]);// Cubic 4-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static inline
  Y* calc_poly_grad_p3 (Y g[4u *3u], const G x[3u]);
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static inline
  Y* calc_poly_grat_p3 (Y g[4u *3u], const G x[3u]);// transposed gradient
  //
  private:
  // Tiling ===================================================================
  // Bars are self-tiling (no other element types involved)
#if 0
  static constexpr fmr::Local_int tile_vert_conn [2] = {0,1};// trivial
#endif
    /*
    Vertex connectivity:
    - For P1, vert_conn sequence: {0,1}, {1,2}, {2,3},...
    - For P2, vert_conn sequence: {0,2}, {2,4}, {4,6},...
    - For P3, vert_conn sequence: {0,3}, {3,6}, {6,9},...

    Node connectivity:
    - For P1, node conn sequence: {0,1}, {1,2}, {2,3},...
    - For P2, node conn sequence: {0,2,1}, {2,4,3}, {4,6,5},...
    - For P3, node conn sequence: {0,3,1,2}, {3,6,4,5}, {6,9,7,8},...
    */
  static constexpr bool calc_tile_vert_conn (fmr::Local_int* conn,
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Local_int p) noexcept;
  static constexpr bool calc_tile_vert_conn_prdc (fmr::Local_int* conn,
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Local_int p) noexcept;
  //
  static constexpr bool calc_tile_node_conn (fmr::Local_int* conn,
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Local_int p) noexcept;
  // Node coordinates ---------------------------------------------------------
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_vert_coor (F* coor,
    fmr::Local_int vert_i, fmr::Local_int vert_n,
    const F* tile_size, const F* part_orig) noexcept;
#if 0
  static constexpr bool calc_tile_node_coor (fmr::Geom_float* coor,
    fmr::Local_int node_i, fmr::Local_int node_n,
    const fmr::Geom_float* tile_size, const fmr::Geom_float* part_orig) noexcept;
#endif
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_elem_vert_coor (F* coor,
    fmr::Local_int elem_i, fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig) noexcept;
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_elem_coor_vert (F* coor,
    fmr::Local_int elem_i, fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig) noexcept;
  //
};//end struct Bars

} } }//end femera::grid::elem:: namespace

#include "Bars.ipp"

//end FEMERA_HAS_GRID_ELEM_BARS_HPP
#endif
