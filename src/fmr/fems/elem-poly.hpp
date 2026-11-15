#ifndef FMR_HAS_FEMS_ELEM_POLY_HPP
#define FMR_HAS_FEMS_ELEM_POLY_HPP

#include "../fmr.hpp"
#include <type_traits>

namespace fmr { namespace fems { namespace elem {
  // Element serendipity shape functions ======================================
  // Encoded by dimension, shape code, and number of nodes.
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
  //NOTE these codes are temporary until proper namespace and classes
  //     are established.
  //
  // N-cube shape functions and gradients =======================================
  // Includes transposed poynomial shape function/gradient
  // These bridge between Geom (input) and Phys (output) classes.
  // These could be dispactched on the polynomial order P (1,2,3),
  // and adapt to arguments of different precision.
  //NOTE the size of f is get_poly_func_size(1) = node_n for order P
  //     the size of g is node_n * elem_d for order P.
  //     Bar reference gradients have one component; x still accepts 3 coordinates.
  //
  template <typename Y, typename G> static constexpr
  Y* calc_poly_func_C1D2N (Y f[2u], const G x[3u]) noexcept;// Linear 2-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C1D2N (Y g[2u], const G[]) noexcept;// node-major, constant
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C1D2N (Y g[2u], const G[]) noexcept;// reference-major
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C1D3N (Y f[3u], const G x[3u]) noexcept;// Quadratic 3-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C1D3N (Y g[3u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C1D3N (Y g[3u], const G x[3u]) noexcept;// reference-major
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C1D4N (Y f[4u], const G x[3u]) noexcept;// Cubic 4-node bar
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C1D4N (Y g[4u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C1D4N (Y g[4u], const G x[3u]) noexcept;// reference-major
  //...and so on up to 5-pt rules
  //
  // Complete simplex nodes follow Tris/Tets vertex, directed edge and face
  // connectivity. Gradients are node-major or reference-major respectively.
  // Reference triangle vertices: (0,0), (1,0), (0,1).
  // Reference tetrahedron adds vertex (0,0,1).
#define FMR_DECLARE_SIMPLEX(NAME, DIM, NODES) \
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr \
  Y* calc_poly_func_##NAME (Y f[NODES], const G x[3u]) noexcept; \
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr \
  Y* calc_poly_grad_node_##NAME (Y g[NODES*DIM], const G x[3u]) noexcept; \
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr \
  Y* calc_poly_node_grad_##NAME (Y g[NODES*DIM], const G x[3u]) noexcept;
  FMR_DECLARE_SIMPLEX(T2D3N,  2, 3)
  FMR_DECLARE_SIMPLEX(T2D6N,  2, 6)
  FMR_DECLARE_SIMPLEX(T2D10N, 2, 10)
  FMR_DECLARE_SIMPLEX(T3D4N,  3, 4)
  FMR_DECLARE_SIMPLEX(T3D10N, 3, 10)
  FMR_DECLARE_SIMPLEX(T3D20N, 3, 20)
#undef FMR_DECLARE_SIMPLEX
  //
  // T2D9N is the incomplete cubic triangle.
  // T2D9N constrains the T2D10N interior value to -sum(vertices)/6
  // + sum(edge nodes)/4; it reproduces P2 but not all cubic polynomials.
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_T2D9N (Y f[9u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_T2D9N (Y g[18u], const G x[3u]) noexcept;// node-major
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_T2D9N (Y g[18u], const G x[3u]) noexcept;// reference-major
  //
  // Quadrilateral and hexahedron nodes follow Quad/Cube vertex and directed
  // edge connectivity. Gradients are node-major or reference-major respectively.
  template <typename Y, typename G> static constexpr
  Y* calc_poly_func_C2D4N (Y f[4u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C2D4N (Y g[8u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C2D4N (Y g[8u], const G x[3u]) noexcept;
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C2D8N (Y f[8u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C2D8N (Y g[16u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C2D8N (Y g[16u], const G x[3u]) noexcept;
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C2D12N (Y f[12u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C2D12N (Y g[24u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C2D12N (Y g[24u], const G x[3u]) noexcept;
  //...and so on up to 5-pt rules
  //
  template <typename Y, typename G> static constexpr
  Y* calc_poly_func_C3D8N (Y f[8u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C3D8N (Y g[24u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C3D8N (Y g[24u], const G x[3u]) noexcept;
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C3D20N (Y f[20u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C3D20N (Y g[60u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C3D20N (Y g[60u], const G x[3u]) noexcept;
  //
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_func_C3D32N (Y f[32u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_grad_node_C3D32N (Y g[96u], const G x[3u]) noexcept;
  template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> static constexpr
  Y* calc_poly_node_grad_C3D32N (Y g[96u], const G x[3u]) noexcept;
  //...and so on up to 5-pt rules

} } }//end fmr::fems::elem namespace

#include "elem-poly.ipp"

#endif
//end FMR_HAS_FEMS_ELEM_POLY_HPP
