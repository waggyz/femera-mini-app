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
  static constexpr fmr::math::Poly poly_type = fmr::math::Poly::Full;
  //
  static constexpr fmr::Local_int sims_d = 3;//TODO remove?
  static constexpr fmr::Dim_int elem_d = 1;
  //
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
  //
  // Methods ====================================================================
  public://TODO make private, accessed through Elem.
  // Bar shape functions and gradients ========================================
  // Includes transposed poynomial shape function/gradient
  // These bridge between Geom (input) and Phys (output) classes.
  // These could be dispactched on the polynomial order P (1,2,3),
  // and adapt to arguments of different precision.
  //NOTE the size of f is get_poly_func_size(1) = node_n for order P
  //     the size of g is get_poly_grad_size(1) = node_n * sims_d for order P
  //     x is sized for 3D, even if not all are dimensions used.
  //
  // Encoded by dimension, shape code, and number of nodes.
  //NOTE these codes are temporary until proper namespace and classes
  //     are established.
  // C: n-Cube (bars, quad, cube) [hyperCube, Box, Orthotope]
  // T: Triangle, Tetrahedron (simplex shapes, with 1DC1 == 1DT1)
  // Y: pYramid, W: Wedge (prism)
  // _C1D2: 1D, 2-node bar (P1)
  // _C1D3: 1D, 3-node bar (P2)
  // _C2D4: 2D, 4-node quad (S1)
  // _C2D8: 2D, 8-node quad (S2)
  // _C3D8: 3D, 8-node cube (S1)
  // _T2D3: 2D, 3-node triangle (P1)
  // _T3D10: 3D, 10-node tetrahedron (P2)
  // _Y3D5: 3D, 5-node pyramid
  // _W3D6: 3D, 6-node wedge (prism)
  template <typename Y, typename G> static constexpr
  Y* calc_poly_func_C1D2N (Y f[2u], const G x[3u]);// Linear 2-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C1D2N (Y g[2u *3u], const G[]);// x[] not used; result is constant
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C1D2N (Y g[2u *3u], const G[]);// transposed gradients
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C1D3N (Y f[3u], const G x[3u]);// Quadratic 3-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C1D3N (Y g[3u *3u], const G x[3u]);
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C1D3N (Y g[3u *3u], const G x[3u]);// transposed gradients
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C1D4N (Y f[4u], const G x[3u]);// Cubic 4-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C1D4N (Y g[4u *3u], const G x[3u]);
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C1D4N (Y g[4u *3u], const G x[3u]);// transposed gradients
  //...and so on up to 5-pt rules
  //
  //NOTE do not do the following.
  // Mashed up with integration rules:
  // _C1D2_GL1: 1D 1-point Gauss-Legendre rule for 2-node bar
  // _C1D3_GL2: 1D 2-point Gauss-Legendre rule for 3-noe bar
  // _C2D4_GL1: 2D 1-point Gauss-Legendre rule for 4-node quad
  // _C2D8_GL4: 2D 4-point Gauss-Legendre rule for 8-node quad
  // _C3D8_GL8: 3D 4-point Gauss-Legendre rule for 8-node cube
  // _T2D3_GL1: 2D 1-point Gauss-Legendre rule for 3-node tris
  // _T2D6_GJ3: 2D 3-point Gauss-Jacobi rule for 6-node singularity tris
  // _T3D4_GL4: 3D 4-point Gauss-Legendre rule for 4-node tetrahedron
  // _Y3D5_GL1: 3D 1-point Gauss-Legendre rule for 5-node pyramid
  // _W3D6_GL1: 3D 1-point Gauss-Legendre rule for 6-node wedge (prism)
};//end struct Bars

} } }//end femera::grid::elem:: namespace

#include "Bars.ipp"

//end FEMERA_HAS_GRID_ELEM_BARS_HPP
#endif
