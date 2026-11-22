#include "../../femera.hpp"
#include "Bars.hpp"
#include "../tile/Bar1.hpp"
#include "../../../fmr/fems/elem-jacs.hpp"
#include "../../../fmr/fems/elem-poly.hpp"
#include "../../../fmr/smat.hpp"

#include <type_traits>

#include <valarray>

#include <gtest/gtest.h>

#undef DEBUG_PRINT

auto  mini = fmr::new_jobs ();

  // These are sized for one partition, not the whole model.
  typedef std::map<fmr::Local_int, fmr::Phys_float> BCmap;// DOF id, value
  typedef std::pair<fmr::Local_int, fmr::Phys_float> BCitem;
  typedef std::vector<BCitem> BClist;

namespace femera { namespace grid { namespace test {

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
        vals[bcs[i].first] = F(bcs[i].second);// Set vals.
    } }
  return n;
}
  template < typename F=fmr::Phys_float> inline
  fmr::Local_int set_bc0 (F* vals, const BClist bcs){// sets listed dof to 0
    const auto n = fmr::Local_int(bcs.size());
    if (n>0) {
      for (fmr::Local_int i = 0; i < n; ++i) {
        vals[bcs[i].first] = F(0);// Set vals.
    } }
  return n;
}
  template < typename F=fmr::Phys_float> inline
  fmr::Local_int sum_bcs (F* vals, const BClist bcs){
  const auto n = fmr::Local_int(bcs.size());
    if (n>0) {
    for (fmr::Local_int i = 0; i < n; ++i) {
      vals[bcs[i].first] += F(bcs[i].second);// Sum vals.
    } }
  return n;
}

} } }//end namespace femera::grid::test =======================================

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  // Prototype unstructured mesh partition full FEA stack.
  //
  // Set mesh parameters. -----------------------------------------------------
  const fmr::Local_int do_elem_n = 9;
  const bool is_unstructured = false;// perturb nodes.//TODO save coor for check.
  //
  const fmr::Geom_float bars_len  = 1.0;// m
  const fmr::Geom_float orig_coor[3] = {0.0, 0.0, 0.0};
  // Set physics parameters. --------------------------------------------------

  const fmr::Phys_float bars_area = 1e-2 * 1e-2;// 1cm x 1cm cross section
  const fmr::Phys_float mtrl_modx = 200e9;// GPa (steel)
  //
  const fmr::Enum_int load_case = 1;
  // 0: no load
  // 1: apply end displacement (only)
  // 2: apply end force (only),
  // 3: pin both ends, apply a displacement near the middle
  // 4: pin both ends, apply a force near the middle
  //    tip_u = end_load * bars_len / (bars_area * mtrl_modx)
  //
  const fmr::Phys_float end_disp = 0.001;// 1 mm
  const fmr::Phys_float end_load = 1e3;// 1 kN tension
  //
  // Set solver parameters. ---------------------------------------------------
  const fmr::Phys_float abs_tol = 1e-6;// absolute tolerance squared
  const fmr::Phys_float rel_tol = 1e-6;// relative tolerance squared
  // ==========================================================================
  using femera::grid::tile::Tile;
  using femera::grid::elem::Elem;
  using femera::grid::tile::Bar1;
  using femera::grid::elem::Bars;
  // Set mesh partition parameters.
  constexpr fmr::Dim_int part_d = 1;
  constexpr fmr::Dim_int part_elem_p = 1;
  std::valarray<fmr::Local_int> part_elem_dims(3);
  part_elem_dims[0] = do_elem_n; part_elem_dims[1] = 1; part_elem_dims[2] = 1;
  std::valarray<fmr::Geom_float> tile_size(3);
  tile_size[0] = bars_len / part_elem_dims[0];
  tile_size[1] = 0.0; tile_size[2] = 0.0;
  // Get derived mesh partition information.
  constexpr Tile<Bar1, part_elem_p> tile;
  const auto part_elem_n = tile.get_tile_part_elem_n (&part_elem_dims[0]);
  const auto part_node_n = tile.get_tile_part_node_n (&part_elem_dims[0]);
  const auto elem_node_n = tile.get_tile_elem_node_n ();// Bars elements
  // Make the mesh.
  std::valarray<fmr::Local_int>
  part_node_conn (std::size_t(part_elem_n) * elem_node_n);
 // Set up iterations ---------------------------------------------------------
  constexpr Elem<Bars, part_elem_p> elem;
  //
//#ifdef DEBUG_PRINT
#if 0
  // Double-check element dimension and node count.
  {// local scope
    bool ok = elem.get_elem_d () == elem_d;
    mini->data->send (fmr::log, ok?" ok ":" :( ", "elem", " dim",
      "%4u", u_int(elem.get_elem_d()));
    //
    ok = elem.get_node_n () == elem_node_n;
    mini->data->send (fmr::log, ok?" ok ":" :( ", "elem", "node",
      "%4u each", elem.get_node_n());
  }
#endif
#if 0
  {// local scope
    bool ok = halo_node_n > 0;
    mini->data->send (fmr::log, ok?" ok ":" :( ", "halo", "node",
      "%u in part", halo_node_n);
  }
#endif
  // Initialize physics -------------------------------------------------------
  // --------------------------------------------------------------------------
  const std::valarray<fmr::Phys_float>
  mtrl_prop(fmr::Phys_float(mtrl_modx * bars_area), 1);// N
  //
  const fmr::Local_int node_dofs_n = part_d;// elasticity only
  // These are kept for correctness check after solve.
  const fmr::Local_int part_dofs_n = part_node_n * node_dofs_n;
  auto part_f = std::valarray<fmr::Phys_float>(part_dofs_n);// Zeroed at iter start.
  auto part_u = std::valarray<fmr::Phys_float>(fmr::Phys_float(0.0), part_dofs_n);
  auto part_p = std::valarray<fmr::Phys_float>(fmr::Phys_float(0.0), part_dofs_n);
  //TODO init part_u to constant strain.
  // Defer temporary solver vector allocation to within iter loop.
  //
  BClist part_disp_bc;
  BClist part_load_bc;
  {// Scope boundary condition maps. These sort and dedupe BCs.
    BCmap maps_disp_bc;
    BCmap maps_load_bc;
    //
    maps_disp_bc[0] = fmr::Phys_float(0.0);// Fix left end.
    //
    switch (load_case) {
      case 0: break;
      case 1: maps_disp_bc[part_dofs_n - 1] = fmr::Phys_float(end_disp);
      break;
      case 2: maps_load_bc[part_dofs_n - 1] = fmr::Phys_float(end_load);
      break;
      case 3: {
        maps_disp_bc[part_dofs_n - 1] = fmr::Phys_float(0.0);
        maps_disp_bc[part_dofs_n / 2] = fmr::Phys_float(end_disp);
      break;
      }
      case 4: {
        maps_disp_bc[part_dofs_n - 1] = fmr::Phys_float(0.0);
        maps_load_bc[part_dofs_n / 2] = fmr::Phys_float(end_load);
      break;
      }
      default: break;
    }
    // Copy maps to lists.
    for (const auto &p : maps_disp_bc) {
      part_disp_bc.emplace_back(p.first, p.second);
    }
    for (const auto &p : maps_load_bc) {
      part_load_bc.emplace_back(p.first, p.second);
    }
  }
  femera::grid::test::set_bcs(&part_u[0], part_disp_bc);
  //const fmr::Phys_float tol = 1e-6;
 // const fmr::Phys_float tol2 = tol * tol;
  // Conjugate gradient is exact when iterated to the number of unknowns.
  const fmr::Local_int iter_max// guard agains overflow.
  = (part_dofs_n < std::numeric_limits<fmr::Local_int>::max() )
  ? fmr::Local_int(part_dofs_n) - fmr::Local_int(part_disp_bc.size()) + 1
  : std::numeric_limits<fmr::Local_int>::max() -1;
  //
    const auto abs_tol2 = abs_tol * abs_tol;
    const auto rel_tol2 = rel_tol * rel_tol;
  fmr::Phys_float res2 = std::numeric_limits<fmr::Phys_float>::max();
  fmr::Phys_float rel2 = std::numeric_limits<fmr::Phys_float>::max();
  fmr::Local_int bad_jac_n = 0;
  fmr::Local_int iter = 0;// Initialization counts as iter 0.
  {// Scope part_jacs_dets ****************************************************
    // Get node coordinates (temporarily).
    //constexpr Elem<Bars, 1> elem_p1;
    //const auto elem_d = tile.get_elem_d ();
    //const auto elem_jacd_size = elem_p1.get_jacs_dets_size (part_elem_n);// WRONG
    //const auto elem_jacd_size = elem_p1.get_jacs_dets_size ();// WRONG
    const fmr::Local_int elem_jacd_size = 2;
#if 0
    printf("elems: %u, elem_jacd_size: %u\n", part_elem_n, elem_jacd_size);
#endif
    //
    auto part_jacs_dets = std::valarray<fmr::Phys_float>
      (std::size_t(part_elem_n * elem_jacd_size));
    //
    {// Scope node_coor
      std::valarray<fmr::Geom_float>
      part_node_coor(std::size_t(part_node_n) * part_d);
      tile.get_tile_mesh (// returns halo_elem_n
        &part_node_conn[0], &part_node_coor[0],//NOTE elems halo sorted, nodes not.
        &tile_size[0],// only [0] used for this tile
        &part_elem_dims[0],
        part_d,
        &orig_coor[0]
      );
FMR_WARN_INLINE_OFF
    bad_jac_n += femera::grid::test::calc_bars_invt_dets<1>( &part_jacs_dets[0],
      &part_node_coor[0], &part_node_conn[0], part_elem_n, part_d);
FMR_WARN_INLINE_ON
    }//end node_coor scope
    // Iterate ================================================================
    const fmr::Local_int elem_intp_n = 1;
    std::valarray<fmr::Phys_float> intp_wgts(1.0, std::size_t(elem_intp_n));
    const std::valarray<fmr::Phys_float> intp_coor
      (0.5, elem_intp_n * elem.get_elem_d ());
    std::valarray<fmr::Phys_float> elem_dN(elem.get_poly_grad_size());
//FMR_WARN_INLINE_OFF
    elem.get_poly_grad_node(&elem_dN[0], &intp_coor[0]);
//FMR_WARN_INLINE_ON
    {
    bool ok = (iter < iter_max) && (bad_jac_n == 0)
      && (res2 > abs_tol2) && (rel2 > rel_tol2)
      && (res2 > 0.0) && (abs_tol2 > 0.0);
    mini->data->send (fmr::log, ok?" ok ":" :( ", " GO ", "init",
      "%4u bad jacd: %u",
      iter, bad_jac_n );
    }
    auto part_r = std::valarray<fmr::Phys_float>(part_dofs_n);
    auto Minv = std::valarray<float>(1.0, part_dofs_n);
    femera::grid::test::set_bc0(&Minv[0], part_disp_bc);
    //
    fmr::Phys_float part_zet2 = 0.0, init_zet2 = 0.0;
    while ((iter < iter_max) && (res2 > abs_tol2) && (rel2 > rel_tol2)) {
      // Init is iter 0.
      part_f = fmr::Phys_float(0.0); // Zero part_f to accumulate sums.
      // ----------------------------------------------------------------------
#if 0
      // Copy loads and displacement BCs into local lists.
      const auto this_disp_bc = part_disp_bc;
      const auto this_load_bc = part_load_bc;
      if (iter == 0) {// Apply them to u. TODO done before entering here.
        femera::grid::test::set_bcs(&part_u[0], this_disp_bc);
      }
      // ----------------------------------------------------------------------
#endif
      // Make local copies of scalar constants.
      const auto halo_elem_n = tile.get_halo_elem_n (&part_elem_dims[0]);
      const auto halo_node_n = tile.get_halo_node_n (&part_elem_dims[0]);
      const auto this_elem_n = part_elem_n;
      const auto conn_n = elem.get_node_n ();//constexpr
      const auto dofs_n = node_dofs_n;
      const auto this_elem_d = elem.get_elem_d ();
      const auto this_part_d = part_d;
      const auto this_intp_n = elem_intp_n;
      const auto this_jacs_dets_size = fmr::Local_int(2);//FIXME: elem.get_jacs_dets_size ();
      const auto this_dofs_n = part_node_n * dofs_n;
      //
      // Constant arrays to copy in.
      const auto this_intp_w = intp_wgts;//std::valarray<fmr::Phys_float>(1.0, this_intp_n);
      const auto this_dN = elem_dN;
      const auto this_jacd = part_jacs_dets;
      //const auto this_det = fmr::Phys_float(0.0);
      //const auto this_jac = std::valarray<fmr::Phys_float>(this_intp_n);
      const auto this_prop = mtrl_prop;
      //
      // Element loop internal arrays
      fmr::Local_int  conn[conn_n];// connectivity buffer
      fmr::Phys_float uval[conn_n * dofs_n];// displacement buffer
      fmr::Phys_float fval[conn_n * dofs_n];// force buffer
      fmr::Phys_float jinv_det[this_jacs_dets_size];
      //
      // physics array: mtrl_prop (small)
      // mesh arrays: part_node_conn, part_jacs_dets
      //
      // input arrays: part_u, part_p
      // output array: part_f : response
      //
      // Do halo elements.
      for (fmr::Local_int elem_i = 0; elem_i < this_elem_n; ++elem_i) {
      //for (fmr::Local_int elem_i = 0; elem_i < halo_elem_n; ++elem_i) {
        for (fmr::Local_int i = 0; i < conn_n; ++i) {// Copy to conn buffer.
          conn[i] = part_node_conn[elem_i * conn_n + i];
        }
#ifdef DEBUG_PRINT
        if (iter < 1 && elem_i < 10) {// Only print the first few elements for debugging.
          mini->data->send (fmr::log, "iter", "mesh", "elem",
            "%4u conn %u %u", elem_i, conn[0], conn[1]);
        }
#endif
        if (iter == 0) {
          for (fmr::Local_int i = 0; i < conn_n; ++i) {
            for (fmr::Local_int d = 0; d < dofs_n; ++d) {// Gather to uval buffer.
              uval[i * dofs_n + d] = part_u[conn[i] * dofs_n + d];
            }
          }
        } else {
          for (fmr::Local_int i = 0; i < conn_n; ++i) {
            for (fmr::Local_int d = 0; d < dofs_n; ++d) {// Gather to uval buffer.
              uval[i * dofs_n + d] = part_p[conn[i] * dofs_n + d];
            }
          }
        }
        // Only one constant geometric Jacobian & determinant for each
        // Bars, Tris, Tets element, regardless of the integration point count.
        for (fmr::Local_int i = 0; i < this_jacs_dets_size; ++i) {
          jinv_det[i] = this_jacd[elem_i * this_jacs_dets_size + i];
        }
        const auto this_det = jinv_det[this_part_d * this_elem_d ];// last value
#if 0
        {
          mini->data->send (fmr::log, "iter", "jacd", "elem",
            "%4u %e %e", elem_i, jinv_det[0], this_det);
        }
#endif
        // Integration point loop internal arrays
        fmr::Phys_float G[dofs_n * conn_n];
        fmr::Phys_float H[dofs_n * this_elem_d];
        fmr::Phys_float S[dofs_n * this_elem_d];
        // --------------------------------------------------------------------
        // Do physics.
        for (fmr::Local_int i = 0; i < conn_n * dofs_n; ++i) {
          fval[i] = fmr::Phys_float(0.0); }
        for (fmr::Local_int ip = 0; ip < this_intp_n; ++ip) {
          // G = Jinv * dN, H =  G * u, S = E * H, f = G^T * S.
          const auto df = this_intp_w[ip] * this_det;
          //
          // G = Jinv * dN
          fmr::math::smat_mmul_ref(&G[0], &jinv_det[0], &this_dN[0],
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
          n = Tile<Bar1, part_elem_p>::get_halo_node(h, &part_elem_dims[0]);
          if (iter < 5) {
            if (iter == 0) {
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
          Tile<Bar1, part_elem_p>::get_halo_node(h, &part_elem_dims[0]);
#endif
        }
      }
    }//end elem loop
    if (iter == 0) {// part solve init scope ==================================
      //const auto this_dofs_n = part_dofs_n;// local copy
      //TODO Initialize iter loop local temporary solver vectors.
      //
      //const auto Minv = std::valarray<float>(1.0, this_dofs_n);
      // Diagonal preconditioner (already inverted & synced)
      //
      //auto part_p = std::valarray<fmr::Phys_float>(this_dofs_n);
      auto part_z = std::valarray<fmr::Phys_float>(this_dofs_n);
      //
      // Copy loads and displacement BCs into local lists.
      //const auto this_disp_bc = part_disp_bc;
      const auto this_load_bc = part_load_bc;
      //
      // Initialize preconditioned conjugate gradient (PCG) solver. -----------
      // f = Y(u) : initial response
      // r = b - f : initial residuals (r)
      //NOTE b is not used again. Is this the RHS?
#if 1
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        // part_r[i] = part_b[i] - part_f[i];
        part_r[i] = -part_f[i];// Assume part_b are applied loads.
      }//TODO pull RHS application out of the iter loop.
      femera::grid::test::sum_bcs(&part_r[0], this_load_bc);// RHS
#endif
      //
      // p = Minv * r : initial gradient descent direction (p)
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        part_p[i] = fmr::Phys_float(Minv[i]) * part_r[i];
      }
      // zet2 = r dot p : reduce initial residual norm.
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        part_zet2 += part_r[i] * part_p[i];
      }
      //TODO Sync sum zet2.
      //
      init_zet2 = part_zet2;// Keep for relative tolerance.
      part_zet2 = init_zet2;
      res2 = part_zet2;
      rel2 = part_zet2 / init_zet2;
      // DONE if already converged.
      if (part_zet2 < abs_tol2) {++iter; break; }// already converged
    }// end part solve init scope
    else {// part solve iteration scope
      //const auto Minv = std::valarray<float>(1.0, this_dofs_n);
      //
      //auto part_r = std::valarray<fmr::Phys_float>(this_dofs_n);
      auto part_z = std::valarray<fmr::Phys_float>(this_dofs_n);
    // Do solver iteration. -------------------------------------------------
      //const auto this_dofs_n = part_dofs_n;// local copy
      // f = Y(p) : already done
      // gamma = p dot f : reduce
      fmr::Phys_float part_gamma = 0.0;
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        part_gamma += part_p[i] * part_f[i];
      }
      // sync sum gamma
      //
      // alpha = zet2 / gamma
      if (part_gamma <= 0.0) {++iter; res2 = part_gamma; break; }
      const auto part_alpha = part_zet2 / part_gamma;
      //
      // r = r - alpha * f : update residuals
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        part_r[i] -= part_alpha * part_f[i];
      }
      // z = Minv * r : preconditioned residual
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        part_z[i] = fmr::Phys_float(Minv[i]) * part_r[i];
      }
      // zeta = r dot z : reduce preconditioned residual norm
      fmr::Phys_float part_zeta = 0.0;
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        part_zeta += part_r[i] * part_z[i];
      }
      // sync sum zeta
      //
      // beta = zeta / zet2
      const auto part_beta = part_zeta / part_zet2;
      //
      // u = u + alpha * p : update solution
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        part_u[i] += part_alpha * part_p[i];
      }
      // p = z + beta * p : update search direction
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        part_p[i] = part_z[i] + part_beta * part_p[i];
      }
      // zeta2 = zeta : update for next iteration
      part_zet2 = part_zeta;
      res2 = part_zet2;
      rel2 = part_zet2 / init_zet2;
    }//end part solve iteration scope
    {
      const auto digits = fmr::math::count_digits (fmr::Local_int(iter_max) * 2);
      fmr::Local_int mod = 1;
      for (fmr::Local_int i = 2; i < digits; ++i) { mod *= 10; }
      if ((iter < 5) || (iter % mod == 0)) {
        bool ok = (res2 < abs_tol2) || (rel2 < rel_tol2);
        const auto istr = iter == 0 ? "init" : "iter";
        mini->data->send (fmr::log, "solv", "PCG", istr,
          "%4u /%4u %.2e,%.2e %2s %.2e,%.2e abs,rel tolerance", iter, iter_max-1,
          sqrt(res2), sqrt(rel2), ok?"< ":">=",
          sqrt(abs_tol2), sqrt(rel_tol2) );
      }
    }
    ++iter;
    }// end iteration while loop
  --iter;// Count is off by one here.
  }// end scope part_jacs_dets
  {
  bool ok = (res2 < abs_tol2) || (rel2 < rel_tol2);
  mini->data->send (fmr::log, ok?" ok ":" :( ", "DONE", "iter",
    "%4u %.2e,%.2e %2s %.2e,%.2e abs,rel tolerance",
    iter, sqrt(res2), sqrt(rel2), ok?"< ":">=", sqrt(abs_tol2), sqrt(rel_tol2) );
  }
  // Check correctness ========================================================
  fmr::Local_int ok = 0;
  const auto numtol = 1e-9;//std::numeric_limits<fmr::Phys_float>::epsilon();
  for (fmr::Local_int i = 0; i < part_dofs_n; ++i) {
    //printf("%4u %0.3e\n", i, part_u[i]);
    switch (load_case) {
    case 0:{// No load
      ok += std::abs(part_u[i] - 0.0) < numtol;
      break;
    }
    case 1: {// 1cm end displacement
      const auto chck = fmr::Phys_float(i)
         * end_disp / fmr::Phys_float(part_node_n - 1);
      //const fmr::Phys_float tip_u = end_disp * bars_len / (bars_area * mtrl_modx);
      ok += std::abs(part_u[i] - chck) < numtol * end_disp;
      break;
    }
    case 2: {// 1 kN end load. (1e3)
      const auto chck = end_load / (mtrl_modx * bars_area) * bars_len;
      ok += std::abs(part_u[i]
      - chck * (fmr::Phys_float(i) / fmr::Phys_float(part_node_n - 1)))
      < numtol;
      break;
    }
    case 3: {// 1 cm end displacement
      ok = 9999;//TODO
      break;
    }
    case 4: {// 1 cm end displacement
      ok = 9999;//TODO
      break;
    }
    default: break;
    }
  }
  mini->data->send (fmr::log, ok==part_dofs_n?" ok ":" :( ", "chck", "disp",
    "%4u / %4u good", ok, part_dofs_n);
  return mini->exit ();
}//end main ()

TEST(BarsPerf, TrivialTest) {
  EXPECT_TRUE(true);
}
namespace femera { namespace test {

} }//end femera::test namespace

#undef DEBUG_PRINT