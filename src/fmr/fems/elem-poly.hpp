#ifndef FMR_HAS_FEMS_ELEM_POLY_HPP
#define FMR_HAS_FEMS_ELEM_POLY_HPP

#include "../fmr.hpp"

namespace fmr { namespace fems { namespace elem {
  // Element shape functions ==================================================
  // N-cube shape functions and gradients =======================================
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
  Y* calc_poly_func_C1D2N (Y f[2u], const G x[3u]) noexcept;// Linear 2-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C1D2N (Y g[2u *3u], const G[]) noexcept;// x[] not used; result is constant
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C1D2N (Y g[2u *3u], const G[]) noexcept;// transposed gradients
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C1D3N (Y f[3u], const G x[3u]) noexcept;// Quadratic 3-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C1D3N (Y g[3u *3u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C1D3N (Y g[3u *3u], const G x[3u]) noexcept;// transposed gradients
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C1D4N (Y f[4u], const G x[3u]) noexcept;// Cubic 4-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C1D4N (Y g[4u *3u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C1D4N (Y g[4u *3u], const G x[3u]) noexcept;// transposed gradients
  //...and so on up to 5-pt rules

} } }//end fmr::fems::elem namespace

#include "elem-poly.ipp"

#endif
//end FMR_HAS_FEMS_ELEM_POLY_HPP

