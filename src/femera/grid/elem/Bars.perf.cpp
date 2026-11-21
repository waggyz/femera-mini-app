#include "../../femera.hpp"
#include "Bars.hpp"
#include "../tile/Bar1.hpp"
#include "../../../fmr/fems/elem-jacs.hpp"
#include "../../../fmr/fems/elem-poly.hpp"
#include "../../../fmr/smat.hpp"

#include <type_traits>

#include <valarray>

#include <gtest/gtest.h>

#define DEBUG_PRINT

auto  mini = fmr::new_jobs ();


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
    bad += fmr::Local_int(abs(this_bad));
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

} } }//end namespace femera::grid::test


fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  // Prototype unstructured mesh partition full FEA stack.
  //
  typedef std::map<fmr::Local_int, fmr::Phys_float> BCmap;
  typedef std::pair<fmr::Local_int, fmr::Phys_float> BCitem;
  typedef std::vector<BCitem> BClist;
  // Set mesh parameters. -----------------------------------------------------
  using femera::grid::tile::Tile;
  using femera::grid::tile::Bar1;
  using femera::grid::elem::Elem;
  using femera::grid::elem::Bars;
  // Set mesh partition parameters.
  constexpr fmr::Dim_int part_d = 1;
  constexpr fmr::Dim_int part_elem_p = 1;
  std::valarray<fmr::Local_int> part_elem_dims(3);
  part_elem_dims[0] = 9; part_elem_dims[1] = 1; part_elem_dims[2] = 1;
  std::valarray<fmr::Geom_float> tile_size(3);
  tile_size[0] = 1.0 / part_elem_dims[0]; tile_size[1] = 0.0; tile_size[2] = 0.0;
  // Get derived mesh partition information.
  constexpr Tile<Bar1, part_elem_p> tile;
  const auto part_elem_n = tile.get_tile_part_elem_n (&part_elem_dims[0]);
  const auto part_node_n = tile.get_tile_part_node_n (&part_elem_dims[0]);
  const auto elem_node_n = tile.get_tile_elem_node_n ();// Bars elements
  // Make the mesh.
  std::valarray<fmr::Local_int>
  part_node_conn (std::size_t(part_elem_n) * elem_node_n);
  // Connectivity: first halo elements, then the remaining elements.
  for (fmr::Local_int el = 0; el < part_elem_n; ++el) {
    const auto h = tile.get_halo_sort_elem (el, &part_elem_dims[0]);
    //const auto el_i = el;
    const auto ok = tile.get_tile_node_conn
      (&part_node_conn[el * elem_node_n], h, &part_elem_dims[0]);
#ifdef DEBUG_PRINT
    if (el < 5) {
      const auto i = el * elem_node_n;
      mini->data->send (fmr::log, ok?" ok ":" :( ", "tile", "elem",
        "%4u conn %u %u (halo sort)",
        el, part_node_conn[i], part_node_conn[i+1]);
    }
#endif
  }
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
  const std::valarray<fmr::Phys_float> mtrl_prop(fmr::Phys_float(1.0), 1);
  //
  const fmr::Local_int node_dofs_n = part_d;
  // These are kept for correctness check after solve.
  const fmr::Local_int part_dofs_n = std::size_t(part_node_n) * node_dofs_n;
  std::valarray<fmr::Phys_float> part_f(part_dofs_n);// Zeroed at iter start.
  std::valarray<fmr::Phys_float> part_u(fmr::Phys_float(0.0), part_dofs_n);//TODO init to constant strain.
  // Defer temporary solver vector allocation to within iter loop.
  //
  BClist part_disp_bc;
  BClist part_load_bc;
  {// Scope boundary condition maps. These sort and dedupe BCs.
    BCmap maps_disp_bc;
    BCmap maps_load_bc;
    maps_disp_bc[0] = fmr::Phys_float(0.0);// Set deisplacemnts.
    maps_load_bc[part_dofs_n - 1] += fmr::Phys_float(1.0);// Sum loads.
    // Copy maps to lists.
    for (const auto &p : maps_disp_bc) {
      part_disp_bc.emplace_back(p.first, p.second);
    }
    for (const auto &p : maps_load_bc) {
      part_load_bc.emplace_back(p.first, p.second);
    }
  }
  const fmr::Phys_float tol = 1e-6;
  const fmr::Phys_float tol2 = tol * tol;
  // Conjugate gradient is exact when iterated to the number of unknowns.
  const fmr::Local_int iter_max
  = part_dofs_n - fmr::Local_int(part_disp_bc.size());
  //
  fmr::Local_int iter = 0;// Initialization counts as iter 0.
  fmr::Phys_float res2 = std::numeric_limits<fmr::Phys_float>::max();
  fmr::Local_int bad_jac_n = 0;
  {// Scope part_jacs_dets *******************************************************
    // Get node coordinates (temporarily).
    constexpr Elem<Bars, 1> elem_p1;
    const auto elem_d = tile.get_elem_d ();
    const auto elem_jacd_size = elem_p1.get_jacs_dets_size (part_elem_n);
    //
    std::valarray<fmr::Phys_float>
    part_jacs_dets (std::size_t(part_elem_n) * elem_jacd_size);
    {// Scope node_coor
      std::valarray<fmr::Geom_float>
      part_node_coor(std::size_t(part_node_n) * part_d);
      for ( fmr::Local_int n=0; n < part_node_n; ++n) {
        fmr::Geom_float buf[4];
        const auto ok = tile.get_tile_part_node_coor
          (buf, n, &part_elem_dims[0], &tile_size[0]);//TODO get rid of buf?
        for (fmr::Dim_int d = 0; d < part_d; ++d) {
          part_node_coor[n * part_d + d] = buf[d];
        }
#ifdef DEBUG_PRINT
        if (n < 5) {
          const auto i = n * part_d;
          mini->data->send (fmr::log, ok?" ok ":" :( ", "tile", "node",
            "%4u coor %f", n, double(part_node_coor[i]));
        }
#endif
      }
FMR_WARN_INLINE_OFF
    bad_jac_n += femera::grid::test::calc_bars_invt_dets<1>( &part_jacs_dets[0],
      &part_node_coor[0], &part_node_conn[0], part_elem_n, part_d);
FMR_WARN_INLINE_ON
    }//end node_coor scope
    // Iterate ================================================================
    const fmr::Local_int elem_intp_n = 1;
    std::valarray<fmr::Phys_float> intp_wgts(0.5, std::size_t(elem_intp_n));
    const std::valarray<fmr::Phys_float> intp_coor
      (0.5, elem_intp_n * elem.get_elem_d ());
    std::valarray<fmr::Phys_float> elem_dN(elem.get_poly_grad_size());
//FMR_WARN_INLINE_OFF
    elem.get_poly_grad_node(&elem_dN[0], &intp_coor[0]);
//FMR_WARN_INLINE_ON
    {
    bool ok = (iter < iter_max) && (bad_jac_n == 0)
      && (res2 > tol2) && (res2 > 0.0) && (tol2 > 0.0);
    mini->data->send (fmr::log, ok?" ok ":" :( ", " GO ", "iter",
      "%4u %.2e %2s %.2e tolerance, bad jacd: %u",
      iter, sqrt(res2), ok?">=":"< ", sqrt(tol2), bad_jac_n );
    }
    while (iter < iter_max && res2 > tol2) {++iter;// Init counts as iter 0.
      part_f = fmr::Phys_float(0.0); // Zero part_f to accumulate sums.
      // Copy loads and displacements into local lists.
      const auto this_disp_bc = part_disp_bc;
      const auto this_load_bc = part_load_bc;
      const auto this_disp_n = fmr::Local_int(this_disp_bc.size());
      const auto this_load_n = fmr::Local_int(this_load_bc.size());
      if (this_load_n > 0) {// Apply point loads.
        for (fmr::Local_int i = 0; i < this_load_n; ++i) {
          part_f[this_load_bc[i].first] += this_load_bc[i].second;// Sum loads.
#ifdef DEBUG_PRINT
          if (iter < 5) {
            mini->data->send (fmr::log, "iter", " BC ", "node",
              "%4u load %f", this_load_bc[i].first,
              double(part_f[this_load_bc[i].first]));
          }
#endif
        }
      }
      if (this_disp_n > 0) {// Apply Dirichlet boundary conditions.
        for (fmr::Local_int i = 0; i < this_disp_n; ++i) {
          part_u[this_disp_bc[i].first] = this_disp_bc[i].second;// Set disps.
 #ifdef DEBUG_PRINT
          if (iter < 5) {
            mini->data->send (fmr::log, "iter", " BC ", "node",
              "%4u disp %f", this_disp_bc[i].first,
              double(part_u[this_disp_bc[i].first]));
          }
#endif
        }
      }
      // Make local copies of constants.
      const auto halo_elem_n = tile.get_halo_elem_n (&part_elem_dims[0]);
      const auto halo_node_n = tile.get_halo_node_n (&part_elem_dims[0]);
      const auto this_elem_n = part_elem_n;
      const auto conn_n = elem.get_node_n ();//constexpr
      const auto dofs_n = node_dofs_n;
      const auto this_elem_d = elem.get_elem_d ();
      const auto this_part_d = part_d;
      const auto this_intp_n = elem_intp_n;
      const auto this_intp_w = intp_wgts;//std::valarray<fmr::Phys_float>(1.0, this_intp_n);
      const auto this_dN = elem_dN;
      //const auto this_det = fmr::Phys_float(0.0);
      //const auto this_jac = std::valarray<fmr::Phys_float>(this_intp_n);
      const auto this_prop = mtrl_prop;
      //
      std::valarray<fmr::Local_int>  conn(conn_n);// connectivity buffer
      std::valarray<fmr::Phys_float> uval(conn_n * dofs_n);// displacement buffer
      std::valarray<fmr::Phys_float> fval(conn_n * dofs_n);// force buffer
      std::valarray<fmr::Phys_float> jinv_det(elem.get_jacs_dets_size ());
      // Do halo elements.
      for (fmr::Local_int elem_i = 0; elem_i < this_elem_n; ++elem_i) {
      //for (fmr::Local_int elem_i = 0; elem_i < halo_elem_n; ++elem_i) {
        for (fmr::Local_int i = 0; i < conn_n; ++i) {// Copy to conn buffer.
          conn[i] = part_node_conn[elem_i * conn_n + i];
        }
#ifdef DEBUG_PRINT
        if (iter < 5 && elem_i < 5) {// Only print the first few elements for debugging.
          mini->data->send (fmr::log, "iter", "mesh", "elem",
            "%4u conn %u %u", elem_i, conn[0], conn[1]);
        }
#endif
        for (fmr::Local_int i = 0; i < conn_n; ++i) {
          for (fmr::Local_int d = 0; d < dofs_n; ++d) {// Gather to uval buffer.
            uval[i * dofs_n + d] = part_u[conn[i] * dofs_n + d];
          }
        }
        // Only one jacobian & determinant for each Bars, Tris, Tets element.
        for (fmr::Local_int i = 0; i < elem.get_jacs_dets_size (); ++i) {
          jinv_det[i] = part_jacs_dets[elem_i * elem.get_jacs_dets_size () + i];
        }
        const auto this_det = jinv_det[this_part_d * elem_d];// last value
        // Do physics.
        fval = fmr::Phys_float(0.0);
        for (fmr::Local_int ip = 0; ip < this_intp_n; ++ip) {
          // G = dN * u, H = Jinv * G, S = E * H, f = G^T * S.
          const auto df = this_intp_w[ip] * this_det;
          //
          // G = dN * u
          std::valarray<fmr::Phys_float> G(conn_n * elem_d);
          fmr::math::smat_mmul_ref(&G[0], &this_dN[0], &uval[0],
            this_elem_d, conn_n, dofs_n);
          // H = Jinv * G
          std::valarray<fmr::Phys_float> H(this_elem_d * this_part_d);
          fmr::math::smat_mmul_ref(&H[0], &jinv_det[0], &G[0],
            this_elem_d, this_part_d, conn_n);
          // S = k * H
          std::valarray<fmr::Phys_float> S(this_elem_d * this_part_d);
#if 0
          for (fmr::Local_int i = 0; i < this_elem_d * this_part_d; ++i) {
            S[i] = this_prop[0] * H[i];
          }
          for (fmr::Local_int i=0; i<(this_elem_d*this_part_d); ++i) {
            S[i]*= df;
          }
#else
          //  S = k * H * dF (merged)
          for (fmr::Local_int i=0; i<(this_elem_d*this_part_d); ++i) {
            S[i] = this_prop[0] * H[i] * df;
          }
#endif
          // f = GT * S * df
          fmr::math::smat_tmul_ref(&fval[0], &G[0], &S[0],
            conn_n, dofs_n, this_elem_d);
        }// end this_intp_n loop
#if 0
            //Physics placeholder
          for (fmr::Local_int n = 0; n < conn_n; ++n) {
            for (fmr::Local_int d = 0; d < dofs_n; ++d) {
              fval[n * dofs_n + d] +=  this_prop[0] * uval[n * dofs_n + d] * df;
            }
          }
#endif
        for (fmr::Local_int i = 0; i < conn_n; ++i) {// Scatter to part_f.
          for (fmr::Local_int d = 0; d < dofs_n; ++d) {
            part_f[conn[i] * dofs_n + d] += fval[i * dofs_n + d];
          }
        }
      if (elem_i == halo_elem_n - 1) {// Synchronize halo nodes.
        for (fmr::Local_int h = 0; h < halo_node_n; ++h) {
          const auto //TODO Move halo_node_list to init.
          n = Tile<Bar1, part_elem_p>::get_halo_node(h, &part_elem_dims[0]);
#ifdef DEBUG_PRINT
          if (iter < 5) {
            mini->data->send (fmr::log, "sync", "halo", "node",
              "%4u force: %f disp: %f (elem %u done)",
              n, part_f[n * dofs_n], part_u[n * dofs_n], elem_i);
          }
#endif
        }
      }
    }//end elem loop
#if 0
      for (fmr::Local_int h = 0; h < halo_node_n; ++h) {
        // Synchronize halo nodes.
      }
      // Do interior elements.
      for (fmr::Local_int elem_i = halo_elem_n; elem_i < this_elem_n; ++elem_i) {
        for (fmr::Local_int i = 0; i < conn_n; ++i) {// Copy to conn buffer.
          conn[i] = part_node_conn[elem_i * conn_n + i];
        }
        for (fmr::Local_int i = 0; i < conn_n; ++i) {
          for (fmr::Local_int d = 0; d < dofs_n; ++d) {// Gather to uval buffer.
            uval[i * dofs_n + d] = part_u[conn[i] * dofs_n + d];
          }
        }
        for (fmr::Local_int i = 0; i < elem.get_jacs_dets_size (); ++i) {
          jinv_det[i] = part_jacs_dets[elem_i * elem.get_jacs_dets_size () + i];
        }
        const auto this_det = jinv_det[part_d * elem_d];// last value
        // Do physics.
        //TODO G = dN * u, H = Jinv * G, S = E * H for real physics
        fval = fmr::Phys_float(0.0);
        for (fmr::Local_int j = 0; j < this_intp_n; ++j) {
          const auto df = this_intp_w[j] * this_det;
          for (fmr::Local_int n = 0; n < conn_n; ++n) {
            for (fmr::Local_int d = 0; d < dofs_n; ++d) {
              fval[n * dofs_n + d] += this_prop[0] * uval[n * dofs_n + d] * df;
            }
          }
        }
        for (fmr::Local_int i = 0; i < conn_n; ++i) {// Scatter to part_f.
          for (fmr::Local_int d = 0; d < dofs_n; ++d) {
            part_f[conn[i] * dofs_n + d] += fval[i * dofs_n + d];
          }
        }
      }
#endif
      const auto this_dofs_n = part_dofs_n;// local copy
      //TODO Initialize iter loop local temporary solver vectors.
      // Do solver iteration.
      for (fmr::Local_int i = 0; i < this_dofs_n; ++i) {
        part_u[i] = part_f[i];// fake solver: just copy f to u
      }
      res2 = iter > 1 ? 0.0 : res2;//TODO Ensure exit for now.
    }// end iteration while loop
  }// end scope part_jacs_dets
  {
  bool ok = res2 < tol2;
  mini->data->send (fmr::log, ok?" ok ":" :( ", "DONE", "iter",
    "%4u %.2e %2s %.2e tolerance",
    iter, sqrt(res2), ok?"< ":">=", sqrt(tol2) );
  }
  // Check correctness ========================================================
  //TODO (fake check for now)
  return mini->exit ();
}//end main ()

TEST(BarsPerf, TrivialTest) {
  EXPECT_TRUE(true);
}
namespace femera { namespace test {

} }//end femera::test namespace

#undef DEBUG_PRINT