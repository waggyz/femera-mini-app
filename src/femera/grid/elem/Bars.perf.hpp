#ifndef FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP
#define FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP

#include "../../femera.hpp"
#include "Bars.hpp"
#include "../tile/Bar1.hpp"// used for MeshPhys class
#include "../../../fmr/fems/elem-jacs.hpp"

#include <stdexcept>

#undef DEBUG_PRINT

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

  template <typename F=fmr::Phys_float, typename G=fmr::Phys_float> inline
  fmr::Local_int set_bcs (F* vals, const std::vector<BCitem<G>>& bcs){
    const auto n = fmr::Local_int(bcs.size());
    if (n>0) {
      for (fmr::Local_int i = 0; i < n; ++i) {
        vals[bcs[i].dof] = F(bcs[i].val);// Set vals.
    } }
  return n;
}
  template <typename F=fmr::Phys_float, typename G=fmr::Phys_float> inline
  fmr::Local_int set_bc0 (F* vals, const std::vector<BCitem<G>>& bcs){// sets listed dof to 0
    const auto n = fmr::Local_int(bcs.size());
    if (n>0) {
      for (fmr::Local_int i = 0; i < n; ++i) {
        vals[bcs[i].dof] = F(0);// Set vals.
    } }
  return n;
}
  template <typename F=fmr::Phys_float, typename G=fmr::Phys_float> inline
  fmr::Local_int sum_bcs (F* vals, const std::vector<BCitem<G>>& bcs){
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
template <typename F=fmr::Phys_float>
class PCGsProto {
public:
PCGsProto() : p(), r(), z(), zet2 (std::numeric_limits<F>::max()) {}
//= default;//default constructor
//
template<typename M=float> inline FMR_ALWAYS_INLINE
F init (F* f, const M* Minv, const BClist& load_bc,
    const fmr::Local_int ndof) {
  init_data (ndof);
  return init_a (f, Minv, load_bc, ndof);
}
template<typename M> inline FMR_ALWAYS_INLINE
F iter (F* f, F* u, const M* Minv, const fmr::Local_int ndof) {
  return iter_a (f, u, Minv, ndof);
}
std::size_t part_exit () {
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
template <typename M> inline FMR_ALWAYS_INLINE
F init_a (F* f, const M* Minv, const BClist& load_bc,
  const fmr::Local_int ndof)
{
  pcgs_kern_init_a_1(&r[0], &f[0], &load_bc, // does inline
    fmr::Local_int(load_bc.size()), ndof);
  pcgs_kern_init_b_ref(&p[0], &Minv[0], &r[0], ndof);
  zet2 = pcgs_kern_init_c_ref(&r[0], &p[0], ndof);
  return zet2;
}
template <typename M> inline
F init_ref(F* f, const M* Minv, const BClist& load_bc,
    const fmr::Local_int ndof) {
  pcgs_kern_init_a_ref(&r[0], &f[0], &load_bc, ndof);// does not inline
  pcgs_kern_init_b_ref(&p[0], &Minv[0], &r[0], ndof);
  zet2 = pcgs_kern_init_c_ref(&r[0], &p[0], ndof);
  return zet2;
}
template <typename M> inline FMR_ALWAYS_INLINE
F iter_a (F* f, F* u, const M* Minv, const fmr::Local_int ndof) {
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
template <typename M> inline FMR_ALWAYS_INLINE
F iter_ref (F* f, F* u, const M* Minv,const fmr::Local_int ndof) {
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
std::valarray<F> p;
std::valarray<F> r;
std::valarray<F> z;
F zet2;
//
std::size_t init_data(const fmr::Local_int ndof) {
  //NOTE valarray inits and resizes to zeros.
  if( p.size() != ndof) {p.resize(ndof);}else{}// zero if needed.
  if( r.size() != ndof) {r.resize(ndof);}else{}
  if( z.size() != ndof) {z.resize(ndof);}else{}
  //NOTE only reallocates when new size exceeds previously allocated capacity
  return ndof * sizeof(F);
}
};//end class PCGsProto






// ============================================================================
//#include "../tile/Bar1.hpp"
template <class T, fmr::Dim_int P=1, typename F=fmr::Phys_float>
class MeshPhys {// This has Mesh and Phys, but Part owns part_f, part_u.
// Structred finite element mesh part.
public:
  MeshPhys() :
  tile(),
  //grid_size(),
  //phys_size(1.0, 1.0, 1.0, 1.0),
  part_node_conn (),
  part_node_coor (),
  part_jacs_dets (),
//  part_Minv (),
  part_mtrl_prop (1.0, 1),
  elem_dN (),
  elem_intp_wgts (1.0, 1),
  elem_intp_coor (),
  //TODO FIXME Make the floowing loop local.
//  G (10), H (10), S (10), jval (10),
  uval (), fval (), conn (),//TODO resize properly in data_init

//  part_disp_bc (),
//  data_load_bc (),
  data_size(0),
  byte_size(0),
  node_ndof (),
  part_d (),
  tile_d (),
  elem_d (),
  elem_intp_n ()
  {}/* : p(), r(), z(), */
  //
  std::size_t exit() {
    // Placeholder implementation for cleanup.
    return sizeof(MeshPhys);
  }
  fmr::Local_int get_node_dofs_n() const {
    return this->node_ndof;
  }
  fmr::Local_int get_part_node_n() const {
    return tile.get_tile_part_node_n (&this->grid_size[0]);
  }
  fmr::Local_int get_part_elem_n() const {
    return tile.get_tile_part_elem_n (&this->grid_size[0]);
  }
  fmr::Local_int get_part_dofs_n() const {
    return tile.get_tile_part_node_n (&this->grid_size[0]) * this->node_ndof;
  }
  std::size_t get_data_size() const { return this->data_size;}// in use
  std::size_t get_byte_size() const { return this->byte_size;}// allocated
//
private:// ====================================================================
  femera::grid::tile::Tile<T, P> tile;
  //
  fmr::Local_int grid_size[4];// lxmxn
  F phys_size[4];// phsical dimensions
  //
  std::valarray<fmr::Local_int> part_node_conn;
  std::valarray<fmr::Geom_float> part_node_coor;
  std::valarray<F> part_jacs_dets;
  // Diagonal preconditioner (already inverted & synced)
//  std::valarray<float> part_Minv;//TODO REMOVE, owned by caller or solver
  std::valarray<F> part_mtrl_prop;
  //
  std::valarray<F> elem_dN;
  std::valarray<F> elem_intp_wgts;
  std::valarray<F> elem_intp_coor;

//  std::valarray<F> G;
//  std::valarray<F> H;
//  std::valarray<F> S;
 // std::valarray<F> jval;
  std::valarray<F> uval;
  std::valarray<F> fval;
  std::valarray<fmr::Local_int> conn;
  //
//  BClist part_disp_bc;//TODO small class to handle BCList, BCmap, set_*_bc,...
//  BClist data_load_bc;
  //
  //fmr::Local_int grid_size[4];//TODO
  //fmr::Local_int phys_size[4];
  //
  std::size_t data_size;// in use
  std::size_t byte_size;// allocated
  //
  fmr::Dim_int node_ndof;
  fmr::Dim_int part_d;// Containing spatial dimension
  fmr::Dim_int tile_d;// Dimension of the tile mesh, tile_dims <= part_d
  fmr::Dim_int elem_d;// Dimension of the element elem_dims <= tile_d
  fmr::Dim_int elem_intp_n;
  //
//  F G[9];// max buffer size
//  F H[9];
//  F S[9];
//  F jval[10];
  //TODO F fval[conn_n * dofs_n], fmr::Local_int  conn[conn_n], 
  // Methods ------------------------------------------------------------------
  bool chck (){
    // Placeholder for checking internal consistency of the mesh and physics.
    // Typically run after init and before resp ().
    return true;//TODO implement actual checks
  }
  std::size_t init_mesh_data() {
    // Resizes internal array (if needed) and fills with zeros or ones.
    // gsize is grid size.  const fmr::Local_int node_dofs_n = this->node_ndof;// elasticity only
  const fmr::Local_int part_node_n = get_part_node_n();
//  const fmr::Local_int part_dofs_n = get_part_dofs_n();
  const fmr::Local_int part_elem_n = get_part_elem_n();
  const fmr::Local_int elem_node_n = tile.get_tile_elem_node_n();
  const fmr::Local_int elem_jacs_dets_size = tile.get_tile_elem_jacs_dets_size();
  //
  std::size_t byte = 0;// bytes used: size()
  std::size_t size = 0;// bytes allocated: sizeof (approximate)
  //
  // member variables: byte_size, data_size, node_ndof,
  // grid_size, phys_size,  part_d, tile_d, elem_d, elem_intp_n
  byte = 2 * sizeof(std::size_t) + 1 * sizeof(fmr::Local_int)
  + 6 * sizeof(fmr::Dim_int);
  size = byte;// approximately the same for now (does not account for padding.)
  //
  byte += 3* sizeof(fmr::Local_int);// grid_size
  size += 4* sizeof(fmr::Local_int);// grid_size
  byte += 3* sizeof(F);// phys_size
  size += 4* sizeof(F);// phys_size
  //
  if (part_node_conn.size() != part_elem_n * elem_node_n) {
    part_node_conn.resize(part_elem_n * elem_node_n);}
  byte += part_node_conn.size() * sizeof (fmr::Local_int);
  size += sizeof(part_node_conn);
  if (part_node_coor.size() != part_node_n * this->part_d) {
    part_node_coor.resize(part_node_n * this->part_d);
  }
  byte += part_node_coor.size() * sizeof (F);
  size += sizeof(part_node_coor);
  if (part_jacs_dets.size() != part_elem_n * elem_jacs_dets_size) {
    part_jacs_dets.resize(part_elem_n * elem_jacs_dets_size);
  }
  byte += part_jacs_dets.size() * sizeof (F);
  size += sizeof(part_jacs_dets);
  //
  // Sizing handled in init_phys.
  byte += part_mtrl_prop.size() * sizeof (F);
  size += sizeof(part_mtrl_prop);
  if (elem_dN.size() != elem_intp_n * elem_node_n * elem_d) {
    elem_dN.resize(elem_intp_n * elem_node_n * elem_d);
  }
  byte += elem_dN.size() * sizeof (F);
  size += sizeof(elem_dN);
  if (elem_intp_wgts.size() != elem_intp_n) {
    elem_intp_wgts.resize(elem_intp_n);
  }
  byte += elem_intp_wgts.size() * sizeof (F);
  size += sizeof(elem_intp_wgts);
  if (elem_intp_coor.size() != elem_intp_n * elem_d) {
    elem_intp_coor.resize(elem_intp_n * elem_d);
  }
  byte += elem_intp_coor.size() * sizeof (F);
  size += sizeof(elem_intp_coor);
  if (uval.size() != part_node_n * node_ndof) {
    uval.resize(part_node_n * node_ndof);
  }
  byte += uval.size() * sizeof(F);
  size += sizeof(uval);
  if (fval.size() != part_node_n * node_ndof) {
    fval.resize(part_node_n * node_ndof);
  }
  byte += fval.size() * sizeof(F);
  size += sizeof(fval);
  if (conn.size() != part_elem_n * elem_node_n) {
    conn.resize(part_elem_n * elem_node_n);
  }
  byte += conn.size() * sizeof(fmr::Local_int);
  size += sizeof(conn);
  //
  this->data_size = byte;
  this->byte_size = size;
  //
  return byte;
  }
public:
  std::size_t init_phys(// set_phys ()?
    const fmr::Local_int nprop, const F* props/*, const F ysize[4]*/) {
      if (nprop != part_mtrl_prop.size()) {
        part_mtrl_prop.resize(nprop, 1.0);
      }
    for (fmr::Local_int i = 0; i < nprop; ++i) {
      part_mtrl_prop[i] = props[i];
      }
  // returns bytes in use by arrays
  //NOTE this amount already included in init_mesh return fvalue.
  return part_mtrl_prop.size() * sizeof(F);
  }
  std::size_t init_mesh(// returns byte size of mesh data arrays
    const fmr::Dim_int ndof,
    const fmr::Local_int gsize[4], const F ysize[4]// total physical size
  ) {
  // returns bytes in use by arrays
  // These are needed to resize arrays.
  this->part_d = 1;//TODO arg to pass in?
  this->elem_intp_n = 1;//TODO get default from tile or element type? arg?
  //this->tile_d = this->tile.get_tile_d();
  this->elem_d = this->tile.get_elem_d();
  this->node_ndof = ndof;
  if (gsize != nullptr) {
    for (int i = 0; i < 3; ++i) { this->grid_size[i] = gsize[i]; }
  }
  if (ysize != nullptr) {
    for (int i = 0; i < 3; ++i) { this->phys_size[i] = ysize[i]; }
  }
  if (this->grid_size[0] == 0) {
    throw std::invalid_argument("MeshPhys requires at least one bar element");
  }
  std::size_t byte = init_mesh_data();// just resizes arrays
  elem_intp_coor = F(0.5);
  elem_intp_wgts = F(1.0);
  Elem<Bars, P, F>::get_poly_grad_node(&elem_dN[0], &elem_intp_coor[0]);
  const F tile_size[3] = {phys_size[0] / F(grid_size[0]), F(0), F(0)};
  //
  fmr::Local_int bad_jac_n = 0;
    {// Scope node_coor//TODO no longer scoped because node_coor lives above.
//TODO optionally clear node_coor when done with it?
#ifdef DEBUG_PRINT
    printf("conn: %lu, coor: %lu size: %f gsize: %u d: %u jacs: %lu\n",
       part_node_conn.size(), part_node_coor.size(), F(phys_size[0]),
       uint(grid_size[0]), uint(part_d), this->part_jacs_dets.size());
#endif
      // Get node coordinates (temporarily).
      tile.get_tile_part_mesh (// returns halo_elem_n
        &part_node_conn[0], &part_node_coor[0],//NOTE elems halo sorted, nodes not.
        &tile_size[0],
        &grid_size[0],
        part_d
      );
#ifdef DEBUG_PRINT
      for (fmr::Local_int el=0; el<part_elem_n; ++el) {
        const auto node1 = part_node_conn[el * elem_node_n + 0];
        const auto node2 = part_node_conn[el * elem_node_n + 1];
        printf("elem: %u/%u: %u:%.2f %u:%.2f\n", el, part_elem_n,
          node1, part_node_coor[node1 * part_d],
          node2, part_node_coor[node2 * part_d]);
      }
#endif
#if 0
      if(is_unstructured && part_node_n > 2) {//TODO externalize this?
        for (fmr::Local_int n = 1; n < part_node_n-1; ++n) {// not the ends
          for (fmr::Dim_int d = 0; d < part_d; ++d) {
            part_node_coor[n * part_d + d] += 0.99 * (bars_len / do_elem_n)
            * (fmr::Geom_float(mini->test->get_rand())
               / mini->test->get_rand_max() - 0.5);// [-0.5,0.5]
          }
        }
      }
#endif
#if 0
      // swap nodes in half the elements in data_node_conn
      for (fmr::Local_int e = 0; e < part_elem_n/2; ++e) {
        std::swap(data_node_conn[e * 2], data_node_conn[e * 2 + 1]);
      }
    mini->data->send(fmr::log, "info", "swap", "elem",
      "Swapped first half of element connectivity");
#endif
FMR_WARN_INLINE_OFF
const auto this_elem_n = this->tile.get_tile_part_elem_n (&grid_size[0]);
    bad_jac_n += femera::grid::test::calc_bars_invt_dets<1>( &part_jacs_dets[0],
      &part_node_coor[0], &part_node_conn[0], this_elem_n, part_d);
FMR_WARN_INLINE_ON
    }//end node_coor scope
#if 0
    if (false) {// apply std::abs() to the determinants.
      for (fmr::Local_int e = 0; e < part_elem_n; ++e) {
        //part_jacs_dets[e * elem_jacd_size + 0]
        //= std::abs(part_jacs_dets[e * elem_jacd_size + 0]);
        part_jacs_dets[e * elem_jacd_size + 1]
        = std::abs(part_jacs_dets[e * elem_jacd_size + 1]);
      }
    }
    // log jacs and dets
    const auto n = (part_elem_n < 10 ? part_elem_n : 10);
    for (fmr::Local_int e = 0; e < n; ++e) {
      mini->data->send(fmr::log, "jacs", "dets", "elem",
        "%4u %0.2e %0.2e", e, part_jacs_dets[e * elem_jacd_size],
        part_jacs_dets[e * elem_jacd_size + 1]);
    }
#endif
//
  return byte;
  }//end init_mesh



public://TODO move back up later **********************************************
  //TODO variations on init and get_part_resp wit and without
  //     unstructured node_coor & jinv_dets overhead for testing.
  bool get_part_resp (F* f, const F* u) {// Caller checks bounds.
    // Get the response (forward pass) for  any given u.
    if (!f || !u) {return false;}

      //TODO temp alias *******************************************************
      auto part_f = f;// temp rename during refactoring
      const auto part_u = u;
      //
      // Avoid making local copies of scalar constants. Here for refactoring
      const auto this_part_d    = part_d;
      const auto dofs_n         = fmr::Dim_int(get_node_dofs_n());
      const auto this_intp_n    = elem_intp_n;
      const auto part_elem_dims = &grid_size[0];
      // Get the rest using tile.get_*.
      const auto this_elem_n    = tile.get_tile_part_elem_n (&part_elem_dims[0]);
      const auto halo_elem_n    = tile.get_halo_elem_n (&part_elem_dims[0]);
      const auto halo_node_n    = tile.get_halo_node_n (&part_elem_dims[0]);
      const auto conn_n         = tile.get_tile_elem_node_n ();
      const auto this_elem_d    = tile.get_elem_d ();
      const auto part_dofs_n
      = tile.get_tile_part_node_n (&part_elem_dims[0]) * dofs_n;
      //
      // Constant arrays to copy in. (Drop the valarray overhead.)
      //const auto part_node_conn = &data_node_conn[0];
      const auto this_jacd      = &part_jacs_dets[0];
//      const auto this_Minv      = &data_Minv[0];
      //const auto elem_dN        = &data_dN[0];
      const auto intp_wgts      = &elem_intp_wgts[0];
      const auto this_prop      = &part_mtrl_prop[0];
//      const auto Minv           = &data_Minv[0];// inverted preconditioner w/BCs
      //
      F G[9];// max buffer size needed (single physics)
      F H[9];//TODO consider compile-time sizing based on Tile elem type?
      F S[9];
      F jval[10];
#if 0
      //TODO uval, fval, conn buffers for element loop
      F uval[conn_n * dofs_n];// displacement buffer
      F fval[conn_n * dofs_n];// force buffer
      fmr::Local_int  conn[conn_n];// connectivity buffer
#endif
      // Zero part_f to accumulate sums.
      for(fmr::Local_int i = 0; i < part_dofs_n; ++i) {
        part_f[i] = F(0.0);
      }
      // Element loop internal arrays
//      F uval[conn_n * dofs_n];// displacement buffer
//      F fval[conn_n * dofs_n];// force buffer
//      fmr::Local_int  conn[conn_n];// connectivity buffer
//      F jval[10];//TODO max buf needed?
      // Element loop =========================================================
      for (fmr::Local_int elem_i = 0; elem_i < this_elem_n; ++elem_i) {
      //
      //for (fmr::Local_int elem_i = 0; elem_i < halo_elem_n; ++elem_i) {
        for (fmr::Local_int i = 0; i < conn_n; ++i) {// Copy to conn buffer.
          conn[i] = part_node_conn[elem_i * conn_n + i];
        }
#ifdef DEBUG_PRINT
        if (this_iter < 1 && elem_i < 10) {// Only print the first few elements for debugging.
          mini->data->send (fmr::log, "iter", "mesh", "elem",
            "%4u conn %u %u", elem_i, conn[0], conn[1]);
        }
#endif
        for (fmr::Local_int i = 0; i < conn_n; ++i) {
          for (fmr::Local_int d = 0; d < dofs_n; ++d) {// Gather to uval buffer.
            uval[i * dofs_n + d] = part_u[conn[i] * dofs_n + d];
          }
        }
        // Only one constant geometric Jacobian & determinant for each
        // Bars, Tris, Tets element, regardless of the integration point count.
        for (fmr::Local_int i = 0; i < tile.get_tile_elem_jacs_dets_size (); ++i) {
          jval[i] = this_jacd[elem_i * tile.get_tile_elem_jacs_dets_size () + i];
        }
        const auto this_det = jval[this_part_d * this_elem_d ];// last value
#if 0
        {
          mini->data->send (fmr::log, "iter", "jacd", "elem",
            "%4u %e %e", elem_i, jval[0], this_det);
        }
#endif
        // Integration point loop internal arrays
//        //F G[dofs_n * conn_n];
//        //F H[dofs_n * this_elem_d];
//        //F S[dofs_n * this_elem_d];
        // --------------------------------------------------------------------
        // Do physics.
        for (fmr::Local_int i = 0; i < conn_n * dofs_n; ++i) {
          fval[i] = F(0.0); }
          //
        for (fmr::Local_int ip = 0; ip < this_intp_n; ++ip) {
          // G = Jinv * dN, H =  G * u, S = E * H, f = G^T * S.
          const auto df = intp_wgts[ip] * this_det;
          //
          // G = Jinv * dN
          fmr::math::smat_mmul_ref(&G[0], &jval[0], &elem_dN[0],
            this_elem_d, dofs_n, conn_n);
          // H = G * u
          fmr::math::smat_mmul_ref(&H[0], &G[0], &uval[0],
            dofs_n, conn_n, this_elem_d);
          // S = k * H
#if 0
          for (fmr::Local_int i = 0; i < this_elem_d * this_elem_d; ++i) {
            S[i] = this_prop[0] * H[i];
          }
          for (fmr::Local_int i=0; i<(this_elem_d*this_elem_d); ++i) {
            S[i]*= df;
          }
#else
          //  S = k * H * dF (merged)
          for (fmr::Local_int i=0; i<(this_elem_d*this_elem_d); ++i) {
            S[i] = this_prop[0] * H[i] * df;
          }
#endif
          // f = GT * S [* df]
          fmr::math::smat_tmul_ref(&fval[0], &G[0], &S[0],
            conn_n, dofs_n, this_elem_d);
        }// end this_intp_n loop
        for (fmr::Local_int i = 0; i < conn_n; ++i) {// Scatter to part_f.
          for (fmr::Local_int d = 0; d < dofs_n; ++d) {
            part_f[conn[i] * dofs_n + d] += fval[i * dofs_n + d];
          }
        }

      if (elem_i == halo_elem_n - 1) {//TODO Synchronize halo nodes.
        for (fmr::Local_int h = 0; h < halo_node_n; ++h) {
#ifdef DEBUG_PRINT
          const auto //TODO Move halo_node_list to init OR renumber nodes.
          n = Tile<Bar1, tile.get_elem_p ()>::get_halo_node(h, &part_elem_dims[0]);
          if (this_iter < 5) {
            if (this_iter == 0) {
              mini->data->send (fmr::log, "sync", "halo", "node",
                "%4u f: %+0.2e u: %+0.2e (elem %u done)",
                n, part_f[n * dofs_n], part_u[n * dofs_n], elem_i);
            } else {
              mini->data->send (fmr::log, "sync", "halo", "node",
                "%4u f: %+0.2e p: %+0.2e (elem %u done)",
                n, part_f[n * dofs_n], part_p[n * dofs_n], elem_i);
            }
         }
#else
          tile.get_halo_node(h, &part_elem_dims[0]);
#endif
        }
      }//end if (elem_i == halo_elem_n - 1)




    }//end elem loop


    return true;
  }
  //
};//end class MeshPhys




} } }//end namespace femera::grid::test =======================================
//

#undef DEBUG_PRINT
//FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP
#endif