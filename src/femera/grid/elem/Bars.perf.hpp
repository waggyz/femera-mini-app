#ifndef FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP
#define FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP

#include "../../femera.hpp"
#include "Bars.hpp"
#include "../../../fmr/fems/elem-jacs.hpp"
#include "../../../fmr/fems/elem-poly.hpp"
#include "../../../fmr/smat.hpp"

namespace femera { namespace grid { namespace test {

// These are sized for one partition, not the whole model.
typedef std::map<fmr::Local_int, fmr::Phys_float> BCmap;// DOF id, value
//typedef std::pair<fmr::Local_int, fmr::Phys_float> BCitem_OLD;
//TODO replace w/struct
template <typename F=fmr::Phys_float>
struct BCitem {
  fmr::Local_int dof;
  F val;
  BCitem (fmr::Local_int n, F v)
  : dof(n), val(v) {}
};
typedef std::vector<BCitem<>> BClist;

using femera::grid::elem::Elem;
using femera::grid::elem::Bars;

  template <fmr::Dim_int P=1,
    typename Y=fmr::Phys_float, typename G=fmr::Geom_float> inline
  fmr::Local_int calc_bars_invt_dets (Y* part_invt_dets,
    const G* part_node_coor,// for full P elem.
    const fmr::Local_int* part_node_conn,
    const fmr::Local_int part_elem_n,
    const fmr::Dim_int part_d) {
  // NOTE this is for Bars elements.
  fmr::Local_int bad = 0;
  Elem<Bars, P> part_elem;
  Elem<Bars, 1> elem_p1;
  const auto elem_d = part_elem.get_elem_d();
  const std::valarray<fmr::Phys_float> intp_coor(0.5, std::size_t(part_d * 1));
  //
  std::valarray<fmr::Phys_float> dN(elem_p1.get_poly_grad_size());
//FMR_WARN_INLINE_OFF
  elem_p1.get_poly_grad_node(&dN[0], &intp_coor[0]);
//FMR_WARN_INLINE_ON
  for (fmr::Local_int el = 0; el < part_elem_n; ++el) {
    std::valarray<fmr::Phys_float>
    coor(std::size_t(part_d * elem_p1.get_node_n()));// buffer
    for (fmr::Local_int n = 0; n < elem_p1.get_node_n(); ++n) {
      for (fmr::Dim_int d = 0; d < part_d; ++d) {
        coor[n * part_d + d] = Y(part_node_coor
          [part_node_conn[el * part_elem.get_node_n() + n] * part_d + d]);
      }
    }
    // Calculate Jacobian.
    std::valarray<fmr::Phys_float> jac(std::size_t(part_d * elem_d));
    fmr::fems::elem::elem_jacs_node_coor_a(
      &jac[0], &coor[0], &dN[0], 1, elem_p1.get_node_n(), part_d, elem_d);
    // Calculate inverse Jacobian and determinant.
    std::valarray<fmr::Phys_float> jac_inv(std::size_t(elem_d * part_d));
    const auto this_bad = fmr::math::m1x1_invt_det_ref
      (&part_invt_dets[el * (part_d * elem_d + 1)], &jac[0]);
    //TODO fmr::math::smat_pinv_col_ref () when elem_d != part_d
    bad += fmr::Local_int(std::abs(this_bad));
#ifdef DEBUG_PRINT
    if (el < 5) {
      const bool ok = (this_bad == 0) &&
        std::abs(part_invt_dets[el * (part_d * elem_d + 1) + 0]
          - 1.0 / part_invt_dets[el * (part_d * elem_d + 1) + 1])
        < std::numeric_limits<fmr::Phys_float>::epsilon();
      mini->data->send (fmr::log, ok?" ok ":" :( ", "jacd", "elem",
        "%4u %f = 1.0 / %f bad: %u / %u", el,
        double(part_invt_dets[el * (part_d * elem_d + 1) + 0]),
        double(part_invt_dets[el * (part_d * elem_d + 1) + 1]),
        this_bad, bad);
    }
#endif
  }//end el loop
  return bad;
}//end calc_bars_invt_dets

  template < typename F=fmr::Phys_float> inline
  fmr::Local_int set_bcs (F* vals, const BClist bcs){
    const auto n = fmr::Local_int(bcs.size());
    if (n>0) {
      for (fmr::Local_int i = 0; i < n; ++i) {
        vals[bcs[i].dof] = F(bcs[i].val);// Set vals.
    } }
  return n;
}
  template < typename F=fmr::Phys_float> inline
  fmr::Local_int set_bc0 (F* vals, const BClist bcs){// sets listed dof to 0
    const auto n = fmr::Local_int(bcs.size());
    if (n>0) {
      for (fmr::Local_int i = 0; i < n; ++i) {
        vals[bcs[i].dof] = F(0);// Set vals.
    } }
  return n;
}
  template < typename F=fmr::Phys_float> inline
  fmr::Local_int sum_bcs (F* vals, const BClist bcs){
  const auto n = fmr::Local_int(bcs.size());
    if (n>0) {
    for (fmr::Local_int i = 0; i < n; ++i) {
      vals[bcs[i].dof] += F(bcs[i].val);// Sum vals.
    } }
  return n;
}
// PCG solver kernels =========================================================
//
template <typename F=fmr::Phys_float> inline
void pcgs_kern_init_a_ref (F* r, const F* f, const BClist* load_bc,
  const fmr::Local_int bc_n, const fmr::Local_int dofs_n) {
  // r = b - f : initial residuals (r)
  //NOTE b (RHS) is not used again.
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    r[i] = -f[i];// Assume part_b are applied loads.
  }
#if 1
  for (fmr::Local_int i = 0; i < bc_n; ++i) {// does inline
    r[(*load_bc)[i].dof] += F((*load_bc)[i].val);// RHS
  }
#else
  sum_bcs(&r[0], *load_bc);// does not inline
#endif
}
template <typename F=fmr::Phys_float, typename G>
void pcgs_kern_init_b_ref (F* p, const G* Minv, const F* r,
  const fmr::Local_int dofs_n) {
  // p = Minv * r : initial gradient descent direction (p)
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    p[i] = F(Minv[i]) * r[i];
  }
}
template <typename F=fmr::Phys_float> inline
F pcgs_kern_init_c_ref (F* r, const F* p,
  const fmr::Local_int dofs_n) {
  // zet2 = r dot p : reduce initial residual norm.
  F zet2 = F(0);
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    zet2 += r[i] * p[i];
  }
  return zet2;
}







} } }//end namespace femera::grid::test =======================================
//
#endif//FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP