#include "Bars.perf.hpp"

#if 0
// included by Bars.perf.hpp
#include "../../femera.hpp"
#include "Bars.hpp"
#include "../../../fmr/fems/elem-jacs.hpp"
#include "../../../fmr/fems/elem-poly.hpp"
#include "../../../fmr/smat.hpp"
#endif

#include "../tile/Bar1.hpp"

#include <type_traits>
#include <valarray>

#include <gtest/gtest.h>

#undef DEBUG_PRINT

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);// Prototype (un)structured partition full FEA stack.
  // Set mesh parameters. -----------------------------------------------------
  //
  const bool is_unstructured = false;// perturb nodes.//TODO save coor to check.
  const fmr::Local_int do_elem_n = 9;
  //
  const fmr::Geom_float bars_len  = 1.0;// 1m long
  //
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
  //
  fmr::Local_int iter_max = 0;// set to zero for max
  // ==========================================================================
  constexpr fmr::Dim_int part_d = 1;
  constexpr fmr::Dim_int part_elem_p = 1;
  // --------------------------------------------------------------------------
  using femera::grid::tile::Tile;
  using femera::grid::elem::Elem;
  using femera::grid::tile::Bar1;
  using femera::grid::elem::Bars;
  //
  std::valarray<fmr::Local_int> data_elem_dims(3);
  data_elem_dims[0] = do_elem_n; data_elem_dims[1] = 1; data_elem_dims[2] = 1;
  std::valarray<fmr::Geom_float> tile_size(3);
  tile_size[0] = bars_len / data_elem_dims[0];
  tile_size[1] = 0.0; tile_size[2] = 0.0;
  // Get derived mesh partition information.
  constexpr Tile<Bar1, part_elem_p> tile;
  const auto part_elem_n = tile.get_tile_part_elem_n (&data_elem_dims[0]);
  const auto part_node_n = tile.get_tile_part_node_n (&data_elem_dims[0]);
  const auto elem_node_n = tile.get_tile_elem_node_n ();// Bars elements
  // Make the mesh.
  std::valarray<fmr::Local_int>
  data_node_conn (std::size_t(part_elem_n) * elem_node_n);
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
  const std::valarray<fmr::Phys_float>
  mtrl_prop(fmr::Phys_float(mtrl_modx * bars_area), 1);// N
  //
  const fmr::Local_int node_dofs_n = part_d;// elasticity only
  const fmr::Local_int part_dofs_n = part_node_n * node_dofs_n;
  auto data_f = std::valarray<fmr::Phys_float>(part_dofs_n);// Zeroed in iter.
  auto data_u = std::valarray<fmr::Phys_float>(fmr::Phys_float(0.0), part_dofs_n);
  //TODO init part_u to constant strain.
  //
  femera::grid::test::BClist part_disp_bc;
  femera::grid::test::BClist data_load_bc;
  {// Scope boundary condition maps. These sort and dedupe BCs.
    femera::grid::test::BCmap maps_disp_bc;
    femera::grid::test::BCmap maps_load_bc;
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
      //part_disp_bc.emplace_back(BCitem<>(p.first, p.second));
    }
    for (const auto &p : maps_load_bc) {
      data_load_bc.emplace_back(p.first, p.second);
    }
  }
  // These are kept for correctness check after solve.
  auto part_f = &data_f[0];
  auto part_u = &data_u[0];
  femera::grid::test::set_bcs(&part_u[0], part_disp_bc);
  //
  // Conjugate gradient is exact when iterated to the number of unknowns.
  if (iter_max == 0) {
    iter_max = (part_dofs_n < std::numeric_limits<fmr::Local_int>::max() )
    ? fmr::Local_int(part_dofs_n) - fmr::Local_int(part_disp_bc.size()) + 1
    : std::numeric_limits<fmr::Local_int>::max() -1;
  }
    const auto abs_tol2 = abs_tol * abs_tol;
    const auto rel_tol2 = rel_tol * rel_tol;
  fmr::Phys_float res2 = std::numeric_limits<fmr::Phys_float>::max();
  fmr::Phys_float rel2 = std::numeric_limits<fmr::Phys_float>::max();
  fmr::Local_int bad_jac_n = 0;
  //
  auto iter_digits = fmr::math::count_digits(iter_max);
  if (iter_digits < 4) { iter_digits = 4; }
  const auto iter_form_str = "%" + std::to_string(iter_digits) + "u";
  fmr::Local_int iter = 0;// Initialization counts as iter 0.
   {// Iteration internal data scope ==========================================
    // For now, pull most array allocation out of the loop.
    //
    const fmr::Local_int elem_jacd_size = tile.get_tile_elem_jacs_dets_size ();
#if 0
    printf("elems: %u, elem_jacd_size: %u\n", part_elem_n, elem_jacd_size);
#endif
    //
    auto part_jacs_dets = std::valarray<fmr::Phys_float>
      (std::size_t(part_elem_n * elem_jacd_size));
    //
    {// Scope node_coor
      // Get node coordinates (temporarily).
      std::valarray<fmr::Geom_float>
      part_node_coor(std::size_t(part_node_n) * part_d);
      tile.get_tile_part_mesh (// returns halo_elem_n
        &data_node_conn[0], &part_node_coor[0],//NOTE elems halo sorted, nodes not.
        &tile_size[0],// only [0] used for Bar1 tile
        &data_elem_dims[0],
        part_d
      );
      if(is_unstructured && part_node_n > 2) {
        for (fmr::Local_int n = 1; n < part_node_n-1; ++n) {// not the ends
          for (fmr::Dim_int d = 0; d < part_d; ++d) {
            part_node_coor[n * part_d + d] += 0.99 * (bars_len / do_elem_n)
            * (fmr::Geom_float(mini->test->get_rand())
               / mini->test->get_rand_max() - 0.5);// [-0.5,0.5]
          }
        }
      }
#if 0
      // swap nodes in half the elements in data_node_conn
      for (fmr::Local_int e = 0; e < part_elem_n/2; ++e) {
        std::swap(data_node_conn[e * 2], data_node_conn[e * 2 + 1]);
      }
    mini->data->send(fmr::log, "info", "swap", "elem",
      "Swapped first half of element connectivity");
#endif
FMR_WARN_INLINE_OFF
    bad_jac_n += femera::grid::test::calc_bars_invt_dets<1>( &part_jacs_dets[0],
      &part_node_coor[0], &data_node_conn[0], part_elem_n, part_d);
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
    // ------------------------------------------------------------------------
    const fmr::Local_int elem_intp_n = 1;
    std::valarray<fmr::Phys_float> data_intp_wgts(1.0, std::size_t(elem_intp_n));
    const std::valarray<fmr::Phys_float> data_intp_coor
      (0.5, elem_intp_n * tile.get_elem_d ());
    std::valarray<fmr::Phys_float> data_dN(elem.get_poly_grad_size());

    elem.get_poly_grad_node(&data_dN[0], &data_intp_coor[0]);
    {
    bool ok = (iter < iter_max) && (bad_jac_n == 0)
      && (res2 > abs_tol2) && (rel2 > rel_tol2)
      && (res2 > 0.0) && (abs_tol2 > 0.0);
    mini->data->send (fmr::log, ok?" ok ":" :( ", "GO! ", "size",
      "%4u dofs %4u node %4u elem (%4u bad/%4u Jacs)",
      part_dofs_n, part_node_n, part_elem_n,
      bad_jac_n, part_elem_n * elem_intp_n );
    }
    auto data_p = std::valarray<fmr::Phys_float>(fmr::Phys_float(0.0), part_dofs_n);
    auto data_r = std::valarray<fmr::Phys_float>(part_dofs_n);
    auto data_z = std::valarray<fmr::Phys_float>(part_dofs_n);
    //
    // Diagonal preconditioner (already inverted & synced)
    auto data_Minv = std::valarray<float>(1.0, part_dofs_n);
    femera::grid::test::set_bc0(&data_Minv[0], part_disp_bc);
    //
    fmr::Phys_float part_zet2 = 0.0, init_zet2 = 0.0;
    //
    // Calculate the total data size for the part.
    fmr::Global_int data_scal_size = 0;
    fmr::Global_int data_node_size = 0;// scales with node/dofs
    fmr::Global_int data_elem_size = 0;// scales with elem count
    //
    // data in and out
    data_scal_size += sizeof(iter);
    data_scal_size += sizeof(res2);
    data_scal_size += sizeof(rel2);
    data_scal_size += sizeof(part_zet2);
    data_scal_size += sizeof(init_zet2);
    data_node_size += sizeof(data_u[0]) * part_node_n * node_dofs_n;
    data_node_size += sizeof(data_f[0]) * part_node_n * node_dofs_n;
    //
    // const data in
    data_scal_size += sizeof(tile) * 0;// static constexpr
    data_scal_size += sizeof(iter_max);
    data_scal_size += sizeof(abs_tol2);
    data_scal_size += sizeof(rel_tol2);
    data_scal_size += sizeof(mtrl_prop[0]) * 1;
    data_scal_size += sizeof(part_dofs_n);
    data_scal_size += sizeof(data_load_bc);// might be in/out
    data_scal_size += sizeof(data_dN[0]) * 2;
    data_scal_size += sizeof(data_intp_wgts[0]) * 1;
    data_node_size += sizeof(data_Minv[0]) * part_node_n * node_dofs_n;
    data_elem_size += sizeof(part_jacs_dets[0]) * part_elem_n * 2;
    data_elem_size += sizeof(data_node_conn[0]) * part_elem_n
    * tile.get_tile_elem_node_n ();
    //
    // internal data
    data_scal_size += sizeof(part_d);
    data_scal_size += sizeof(node_dofs_n);
    data_scal_size += sizeof(elem_intp_n);
    data_scal_size += sizeof(data_elem_dims[0]) * 1;
    // this_elem_n, halo_elem_n, halo_node_n, conn_n, this_jacs_dets_size
    data_scal_size += sizeof(fmr::Local_int) * 5;
    data_scal_size += sizeof(fmr::Dim_int) * 1;// this_elem_d
    data_node_size += sizeof(data_p[0]) * part_node_n * node_dofs_n;
    data_node_size += sizeof(data_r[0]) * part_node_n * node_dofs_n;
    data_node_size += sizeof(data_z[0]) * part_node_n * node_dofs_n;
    //
    const auto part_data_size = data_scal_size + data_node_size + data_elem_size;
    mini->data->send (fmr::log, "Bars", "PCGs", "data",
      "%s: %s + %s + %s",
      fmr::form::si_unit (part_data_size, "B").c_str(),
      fmr::form::si_unit (data_scal_size, "B").c_str(),
      fmr::form::si_unit (data_node_size / part_node_n, "B/node").c_str(),
      fmr::form::si_unit (data_elem_size / part_elem_n, "B/elem").c_str());
    // fmr::form::si_unit (perf.get_byte (), "B   ").c_str(),
    // Iterate ================================================================
    while ((iter < iter_max) && (res2 > abs_tol2) && (rel2 > rel_tol2)) {
      // Init is iter 0.
      // ----------------------------------------------------------------------
#if 0
      // Copy loads and displacement BCs into local lists.
      const auto this_disp_bc = part_disp_bc;// not used in iter loop
      const auto part_load_bc = data_load_bc;// only used once and copied in init
#endif
      // Make local copies of scalar constants.
      const auto this_part_d    = part_d;
      const auto dofs_n         = node_dofs_n;
      const auto this_intp_n    = elem_intp_n;
      const auto part_elem_dims = &data_elem_dims[0];
      // Get the rest using tile.get_*.
      const auto this_elem_n    = tile.get_tile_part_elem_n (&part_elem_dims[0]);
      const auto halo_elem_n    = tile.get_halo_elem_n (&part_elem_dims[0]);
      const auto halo_node_n    = tile.get_halo_node_n (&part_elem_dims[0]);
      const auto conn_n         = tile.get_tile_elem_node_n ();
      const auto this_elem_d    = tile.get_elem_d ();
      const auto this_dofs_n
      = tile.get_tile_part_node_n (&part_elem_dims[0]) * dofs_n;
      const auto this_jacs_dets_size = tile.get_tile_elem_jacs_dets_size ();
      //
      // Constant arrays to copy in. (Drop the valarray overhead.)
      const auto part_node_conn = &data_node_conn[0];
      const auto this_jacd      = &part_jacs_dets[0];
      const auto elem_dN        = &data_dN[0];
      const auto intp_wgts      = &data_intp_wgts[0];
      const auto this_prop      = &mtrl_prop[0];
      const auto Minv           = &data_Minv[0];// inverted preconditioner w/BCs
      // PCG working vectors
      auto part_p = &data_p[0];
      auto part_r = &data_r[0];
      //
      // Zero part_f to accumulate sums.
      for(fmr::Local_int i = 0; i < part_dofs_n; ++i) {
        part_f[i] = fmr::Phys_float(0.0);
      }
      // Element loop internal arrays
      fmr::Phys_float uval[conn_n * dofs_n];// displacement buffer
      fmr::Phys_float fval[conn_n * dofs_n];// force buffer
      fmr::Local_int  conn[conn_n];// connectivity buffer
      fmr::Phys_float jinv_det[this_jacs_dets_size];
      // Element loop =========================================================
      for (fmr::Local_int elem_i = 0; elem_i < this_elem_n; ++elem_i) {
      //
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
          const auto df = intp_wgts[ip] * this_det;
          //
          // G = Jinv * dN
          fmr::math::smat_mmul_ref(&G[0], &jinv_det[0], &elem_dN[0],
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
      using namespace femera::grid::test;
      //
      // Copy loads and displacement BCs into local lists.
      const auto part_load_bc = data_load_bc;// only used once
      //
      // Initialize preconditioned conjugate gradient (PCG) solver. -----------
      // Every step, even when trivial, is a defined kernel, so that later
      // merges track each step.
      // E.g., pcgs_kern_iter_cde_ref merges steps c, d, and e into one kernel
      // (reference implementation).
      //
      // f = Y(u) : initial response assumed already done.
      //
      // r = b - f : initial residuals (r)
      //NOTE b (RHS) is not used again.
#if 1
      pcgs_kern_init_a_1(&part_r[0], &part_f[0], &part_load_bc,
        fmr::Local_int(part_load_bc.size()), this_dofs_n);// does inline
#else
      pcgs_kern_init_a_ref(&part_r[0], &part_f[0], &part_load_bc,
        this_dofs_n);// does not inline
#endif
      pcgs_kern_init_b_ref(&part_p[0], &Minv[0], &part_r[0], this_dofs_n);
      part_zet2 = pcgs_kern_init_c_ref(&part_r[0], &part_p[0], this_dofs_n);
      //
      //TODO Sync sum zet2.
      //
      init_zet2 = part_zet2;// Keep for relative tolerance.
      res2 = part_zet2;
      rel2 = part_zet2 / init_zet2;
      // DONE if already converged.
      if (part_zet2 < abs_tol2) {++iter; break; }// already converged
    }// end part solve init scope
    else {// part solve iteration scope
#define MERGE_PCG_ITER_CDE_GH
      using namespace femera::grid::test;
      //
      auto part_z = &data_z[0];
    // Do solver iteration. ---------------------------------------------------
    //
    // f = Y(p) : assumed already done
    const fmr::Phys_float// gamma = p dot f : reduce
    part_gamma = pcgs_kern_iter_a_ref(&part_p[0], &part_f[0], this_dofs_n);
      //
      //TODO sync sum gamma
      //
      if (part_gamma <= 0.0) {++iter; res2 = part_gamma; break; }//TODO remove?
      //
      // alpha = zet2 / gamma
      const auto part_alpha = pcgs_kern_iter_b_ref (part_zet2, part_gamma);
#ifdef MERGE_PCG_ITER_CDE_GH
      // r = r - alpha * f : update residuals
      // z = Minv * r : preconditioned residual
      // zeta = r dot z : reduce preconditioned residual norm
      const fmr::Phys_float part_zeta = pcgs_kern_iter_cde_ref
        (&part_r[0], &part_z[0], &part_f[0], &Minv[0], part_alpha, this_dofs_n);
#else
      // r = r - alpha * f : update residuals
      pcgs_kern_iter_c_ref (&part_r[0], &part_f[0], part_alpha, this_dofs_n);
      // z = Minv * r : preconditioned residual
      pcgs_kern_iter_d_ref (&part_z[0], &Minv[0], &part_r[0], this_dofs_n);
      // zeta = r dot z : reduce preconditioned residual norm
      const fmr::Phys_float part_zeta
      = pcgs_kern_iter_e_ref(&part_r[0], &part_z[0], this_dofs_n);
#endif
      //
      //TODO sync sum zeta
      //
      // beta = zeta / zet2
      const fmr::Phys_float
      part_beta = pcgs_kern_iter_f_ref (part_zeta, part_zet2);
#ifdef MERGE_PCG_ITER_CDE_GH
      // u = u + alpha * p : update solution
      // p = z + beta * p : update search direction
     pcgs_kern_iter_gh_ref(&part_u[0], &part_p[0],
       &part_z[0], part_alpha, part_beta, this_dofs_n);
#else
      // u = u + alpha * p : update solution
      pcgs_kern_iter_g_ref (&part_u[0], &part_p[0], part_alpha, this_dofs_n);
      // p = z + beta * p : update search direction
      pcgs_kern_iter_h_ref (&part_p[0], &part_z[0], part_beta, this_dofs_n);
#endif
      // zeta2 = zeta : update for next iteration
      part_zet2 = pcgs_kern_i_ref (part_zeta);
      //
#undef MERGE_PCG_ITER_CDE_GH
      // iteration residuals
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
        if (iter == 0) {
          const auto format = iter_form_str + " /" + iter_form_str
          + " %.2e,%.2e %2s %.2e,%.2e rel,abs tol";
          mini->data->send (fmr::log, "Bars", "PCGs", istr,
            format.c_str(), iter, iter_max-1,
            sqrt(rel2), sqrt(res2), ok?"< ":">=",
            sqrt(rel_tol2), sqrt(abs_tol2) );
        } else {
          const auto format = iter_form_str + " /" + iter_form_str + " %.2e";
          mini->data->send (fmr::log, "Bars", "PCGs", istr,
            format.c_str(), iter, iter_max-1, sqrt(rel2) );
        }
      }
    }
    ++iter;
    }// end iteration while loop
  --iter;// Count is off by one here.
  }// end scope part_jacs_dets
  {
  const auto format = iter_form_str + " /" + iter_form_str
  + " %.2e,%.2e %2s %.2e,%.2e rel,abs tol";
  bool ok = (res2 < abs_tol2) || (rel2 < rel_tol2);
  mini->data->send (fmr::log, ok?" ok ":" :( ", "DONE", "iter",
    format.c_str(), iter, iter_max-1,
    sqrt(rel2), sqrt(res2), ok?"< ":">=", sqrt(rel_tol2), sqrt(abs_tol2) );
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
    "%4u / %4u < %0.2e error", ok, part_dofs_n, numtol);
  return mini->exit ();
}//end main ()

TEST(BarsPerf, TrivialTest) {
  EXPECT_TRUE(true);
}
namespace femera { namespace test {

} }//end femera::test namespace

#undef DEBUG_PRINT