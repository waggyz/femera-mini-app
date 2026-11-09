#ifndef FMR_HAS_FEMS_ELEM_HPP
#define FMR_HAS_FEMS_ELEM_HPP

#include "../fmr.hpp"
#include "../smat.hpp"

namespace fmr { namespace fems { namespace elem {
  // Element functions ========================================================
  //F calc_bbox_size // 1D:length, 2D:area, 3D:volume
  //  (const F* node_coor, const fmr::Dim_int dims=D) noexcept;// needs node_n
  //
  // Element Jacobian and determinant calculations
  //NOTE Although element Jacobians and determinants are purely geometric,
  //     these calculations span geometry and physics.
  //     Convert coordinates to Phys_float first, if needed.
  //
  template <typename F=Phys_float>
  inline void calc_jacs_csoa_ref (F* jacs,
    // Calculates Jacobians with nodes provided as a struct of arrays (nsoa)
    const F* coor_node,// x1,x2,..., y1,y2,..., z1,z2,...
    const F* poly_grat,// transpose gradients at integration points
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n,
    const fmr::Local_int sims_d)
    noexcept;
  template <typename F=Phys_float>
  inline void calc_jacs_caos_ref (F* jacs,
    // Calculates Jacobians with nodes provided as an array of structs (naos)
    const F* node_coor,// x1,y1,z1, x2,y2,z2,...
    const F* poly_grat,// transposed gradients at integration points
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n,
    const fmr::Local_int sims_d)
    noexcept;
  //
  template <typename F=Phys_float>
  inline void calc_jacs_caos_a (F* jacs,
    // Calculates Jacobians with vertices provided as an array of structs (AoS)
    const F* vert_node,// x1,y1,z1, x2,y2,z2,...
    const F* poly_grad,// per integration point: dimension-major, then vertex
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n,
    const fmr::Local_int sims_d)
    noexcept;

} } }//end fmr::fems::elem namespace

#include "elem.ipp"

#endif
//end FMR_HAS_FEMS_ELEM_HPP

