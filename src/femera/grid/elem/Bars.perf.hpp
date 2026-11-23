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
  const fmr::Local_int dofs_n) {
  // r = b - f : initial residuals (r)
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    r[i] = -f[i];// Assume part_b are applied loads.
  }
  sum_bcs(&r[0], *load_bc);// does not inline
}
template <typename F=fmr::Phys_float> inline
void pcgs_kern_init_a_1 (F* r, const F* f, const BClist* load_bc,
  const fmr::Local_int bc_n, const fmr::Local_int dofs_n) {
  // r = b - f : initial residuals (r)
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    r[i] = -f[i];// Assume part_b are applied loads.
  }
    if (bc_n > 0) {
      for (fmr::Local_int i = 0; i < bc_n; ++i) {// does inline
        r[(*load_bc)[i].dof] += F((*load_bc)[i].val);// RHS
    } }
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
template <typename F=fmr::Phys_float> inline
F pcgs_kern_iter_a_ref(const F* p, const F* f, const fmr::Local_int dofs_n) {
  // gamma = p dot f : reduce
  F gamma = F(0);
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    gamma += p[i] * f[i];
  }
  return gamma;
}
template <typename F=fmr::Phys_float> inline
F pcgs_kern_iter_b_ref(const F zet2, const F gamma) {
  // alpha = zet2 / gamma
  return zet2 / gamma;
}
template <typename F=fmr::Phys_float> inline
void pcgs_kern_iter_c_ref (F* r, const F* f, const F alpha,
  const fmr::Local_int dofs_n) {
  // r = r - alpha * f : update residuals
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    r[i] -= alpha * f[i];
  }
}
template <typename F=fmr::Phys_float, typename G> inline
void pcgs_kern_iter_d_ref (F* z, const G* Minv, const F* r,
  const fmr::Local_int dofs_n) {
  // z = Minv * r : preconditioned residual
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    z[i] = F(Minv[i]) * r[i];
  }
}
template <typename F=fmr::Phys_float> inline
F pcgs_kern_iter_e_ref(const F* r, const F* z, const fmr::Local_int dofs_n) {
  // zeta = r dot z : reduce preconditioned residual norm
  F zeta = F(0);
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    zeta += r[i] * z[i];
  }
  return zeta;
}
template <typename F=fmr::Phys_float, typename G> inline
F pcgs_kern_iter_cde_ref (F* r, F* z, const F* f, const G* Minv, const F alpha,
  const fmr::Local_int dofs_n) {
  // Merge steps c, d, and e: update residuals, preconditioned residual, and zeta.
  // r = r - alpha * f : update residuals
  // z = Minv * r : preconditioned residual
  // zeta = r dot z : reduce preconditioned residual norm
  F zeta = F(0);
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    r[i] -= alpha * f[i];
    z[i] = F(Minv[i]) * r[i];
    zeta += r[i] * z[i];
  }
  return zeta;
}
template <typename F=fmr::Phys_float> inline
F pcgs_kern_iter_f_ref(const F zeta, const F zet2) {
  // beta = zeta / zet2
  return zeta / zet2;
}
template <typename F=fmr::Phys_float> inline
void pcgs_kern_iter_g_ref (F* u, const F* p, const F alpha,
  const fmr::Local_int dofs_n) {
  // u = u + alpha * p : update solution
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    u[i] += alpha * p[i];
  }
}
template <typename F=fmr::Phys_float> inline
void pcgs_kern_iter_h_ref (F* p, const F* z, const F beta,
  const fmr::Local_int dofs_n) {
  // p = z + beta * p : update search direction
  for (fmr::Local_int i = 0; i < dofs_n; ++i) {
    p[i] = z[i] + beta * p[i];
  }
}
template <typename F=fmr::Phys_float> inline
void pcgs_kern_iter_gh_ref (F* u, F* p,
   const F* z, const F alpha, const F beta, const fmr::Local_int dofs_n) {
    // u = u + alpha * p : update solution
    for (fmr::Local_int i = 0; i < dofs_n; ++i) {
      u[i] += alpha * p[i];
    // p = z + beta * p : update search direction
      p[i] = z[i] + beta * p[i];
    }
   }
template <typename F=fmr::Phys_float> inline
F pcgs_kern_i_ref(const F zeta) {
  // zet2 = zeta : update for next iteration
  return zeta;
}
// ============================================================================
template <class T, typename F=fmr::Phys_float>
class PCGs {
public:
F init (F* f, const F* Minv, const BClist& load_bc,
    const fmr::Local_int ndof) {
  init_data(ndof);
  return init_a(f, Minv, load_bc, ndof);
}
F iter (F* f, F* u, const F* Minv, const fmr::Local_int ndof) {
  return iter_a(f, u, Minv, ndof);
}
std::size_t exit () {
  const std::size_t bytes = sizeof(F) * (p.size()  + r.size() + z.size());
  p.resize(0);// Resize to zero should free memory.
  r.resize(0);
  z.resize(0);
  return bytes;
}
F* ptr_p() { return &p[0];
}
F get_res2() const { return zet2;
}
std::size_t get_byte_size() const {//NOTE includes one extra F for zet2
  return sizeof(F) * (p.size() + r.size() + z.size() + 1);
}
// Kernel variations ----------------------------------------------------------
template <typename G=fmr::Geom_float> inline
F init_a (F* f, const G* Minv, const BClist& load_bc,
    const fmr::Local_int ndof) {
  pcgs_kern_init_a_1(&r[0], &f[0], &load_bc,
    fmr::Local_int(load_bc.size()), ndof); // does inline
  pcgs_kern_init_b_ref(&p[0], &Minv[0], &r[0], ndof);
  zet2 = pcgs_kern_init_c_ref(&r[0], &p[0], ndof);
  return zet2;
}
template <typename G=fmr::Geom_float> inline
F init_ref(F* f, const G* Minv, const BClist& load_bc,
    const fmr::Local_int ndof) {
  pcgs_kern_init_a_ref(&r[0], &f[0], &load_bc, ndof);// does not inline
  pcgs_kern_init_b_ref(&p[0], &Minv[0], &r[0], ndof);
  zet2 = pcgs_kern_init_c_ref(&r[0], &p[0], ndof);
  return zet2;
}
template <typename G=fmr::Geom_float> inline
F iter_a(F* f, F* u, const G* Minv, const fmr::Local_int ndof) {
  // gamma = p dot f : reduce
  const F gamma = pcgs_kern_iter_a_ref(&p[0], &f[0], ndof);
  //
  //TODO sync sum gamma
  //
  //if (gamma <= 0.0) {++iter; res2 = gamma; break; }//TODO remove?
  //
  // alpha = zet2 / gamma
  const auto alpha = pcgs_kern_iter_b_ref (zet2, gamma);
  // r = r - alpha * f : update residuals
  // z = Minv * r : preconditioned residual
  // zeta = r dot z : reduce preconditioned residual norm
  const auto zeta = pcgs_kern_iter_cde_ref (&r[0], &z[0], &f[0], &Minv[0],
    alpha, ndof);
  //
  //TODO sync sum zeta
  //
  // beta = zeta / zet2
  //const fmr::Phys_float zeta = zeta;
  const auto beta = pcgs_kern_iter_f_ref (zeta, zet2);
  // u = u + alpha * p : update solution
  // p = z + beta * p : update search direction
  pcgs_kern_iter_gh_ref(&u[0], &p[0], &z[0], alpha, beta, ndof);
  // zeta2 = zeta : update for next iteration
  zet2 = pcgs_kern_i_ref (zeta);
  return zet2;
}
template <typename G=fmr::Geom_float> inline
F iter_ref(F* f, F* u, const G* Minv, const fmr::Local_int ndof) {
  // gamma = p dot f : reduce
  const auto gamma = pcgs_kern_iter_a_ref(&p[0], &f[0], ndof);
  //
  //TODO sync sum gamma
  //
  //if (gamma <= 0.0) {++iter; res2 = gamma; break; }//TODO remove?
  //
  // alpha = zet2 / gamma
  const auto alpha = pcgs_kern_iter_b_ref (zet2, gamma);
  // r = r - alpha * f : update residuals
  pcgs_kern_iter_c_ref (&r[0], &f[0], alpha, ndof);
  // z = Minv * r : preconditioned residual
  pcgs_kern_iter_d_ref (&z[0], &Minv[0], &r[0], ndof);
  // zeta = r dot z : reduce preconditioned residual norm
  const auto zeta = pcgs_kern_iter_e_ref(&r[0], &z[0], ndof);
  //
  //TODO sync sum zeta
  //
  // beta = zeta / zet2
  const auto beta = pcgs_kern_iter_f_ref (zeta, zet2);
  // u = u + alpha * p : update solution
  pcgs_kern_iter_g_ref (&u[0], &p[0], alpha, ndof);
    // p = z + beta * p : update search direction
  pcgs_kern_iter_h_ref (&p[0], &z[0], beta, ndof);
  // zeta2 = zeta : update for next iteration
  zet2 = pcgs_kern_i_ref (zeta);
  return zet2;
}
private:// ====================================================================
F zet2;
std::valarray<F> p;
std::valarray<F> r;
std::valarray<F> z;
std::size_t init_data(const fmr::Local_int ndof) {
  //NOTE valarray inits and resizes to zeros.
  if( p.size() != ndof) {p.resize(ndof);}else{}// zero if needed.
  if( r.size() != ndof) {r.resize(ndof);}else{}
  if( z.size() != ndof) {z.resize(ndof);}else{}
  //NOTE only reallocates when new size exceeds previously allocated capacity
  return ndof * sizeof(F);
}
};//end class PCGs






} } }//end namespace femera::grid::test =======================================
//
#endif//FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP