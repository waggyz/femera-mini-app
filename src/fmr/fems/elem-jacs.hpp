#ifndef FMR_HAS_FEMS_JACS_HPP
#define FMR_HAS_FEMS_JACS_HPP

#include "../fmr.hpp"
#include "../smat.hpp"

namespace fmr { namespace fems { namespace elem {
  // Element functions ========================================================
  //F calc_bbox_size // 1D:length, 2D:area, 3D:volume
  //  (const F* node_coor, const fmr::Local_int dims=D) noexcept;// needs node_n
  //
  // Element Jacobian calculations. Jacobians are row-major sims_d x elem_d,
  // with integration points stored consecutively.
  // Callers provide non-null buffers sized for the documented layouts and
  // dimensions satisfying 0 < elem_d <= sims_d.
  //NOTE Although element Jacobians and determinants are purely geometric,
  //     these calculations span geometry and physics.
  //     Convert coordinates to Phys_float first, if needed.
  //
  template <typename F=Phys_float>
  inline void elem_jacs_coor_node_ref (F* jacs,
    // Calculates Jacobians with nodes provided as a struct of arrays (SoA)
    const F* coor_node,// x1,x2,..., y1,y2,..., z1,z2,...
    const F* poly_grad,// [ip][node][reference-coordinate]
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n,
    const fmr::Dim_int sims_d,
    const fmr::Dim_int elem_d)
    noexcept;
  template <typename F=Phys_float>
  inline void elem_jacs_node_coor_ref (F* jacs,
    // Calculates Jacobians with nodes provided as an array of structs (AoS)
    const F* node_coor,// x1,y1,z1, x2,y2,z2,...
    const F* poly_grad,// [ip][node][reference-coordinate]
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n,
    const fmr::Dim_int sims_d,
    const fmr::Dim_int elem_d)
    noexcept;
  //
  template <typename F=Phys_float>
  inline void elem_jacs_node_coor_a (F* jacs,
    // Calculates Jacobians with vertices provided as an array of structs (AoS)
    const F* vert_node,// x1,y1,z1, x2,y2,z2,...
    const F* poly_grad,// [ip][reference-coordinate][node]
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n,
    const fmr::Dim_int sims_d,
    const fmr::Dim_int elem_d)
    noexcept;

} } }//end fmr::fems::elem namespace

#include "elem-jacs.ipp"

#endif
//end FMR_HAS_FEMS_ELEM_HPP
