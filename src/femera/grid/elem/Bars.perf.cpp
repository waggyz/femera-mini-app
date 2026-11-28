#include "Bars.perf.hpp"

#include "../tile/Bar1.hpp"

#include <type_traits>
#include <valarray>

#include <gtest/gtest.h>

#undef DEBUG_PRINT

auto  mini = fmr::new_jobs ();

    inline void send_perf_results (std::string label, fmr::Enum_int load_case,
    fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>& perf,
    fmr::Perf_float this_time,
    fmr::Local_int iter_count, uint test_count)  {
      mini->data->send (fmr::out, "elem", "Bars", label.c_str(),
        "(%i) %s: %s/%s, iter: %u",
        load_case,
        fmr::form::si_unit (perf.get_busy_unit_speed (), "dofs/s").c_str(),
        fmr::form::si_unit (perf.get_unit (), perf.get_unit_name ()).c_str(),
        fmr::form::si_unit (this_time, "s").c_str(),
        iter_count);
      mini->data->send (fmr::out, "elem", "Bars", label.c_str(),
        "(%i) %s: %s/%s, test: %u",
        load_case,
        fmr::form::si_unit (perf.get_busy_flop_speed (), "flop/s").c_str(),
        fmr::form::si_unit (perf.get_flop (), "flop").c_str(),
        fmr::form::si_unit (this_time, "s").c_str(),
        test_count);
      mini->data->send (fmr::out, "elem", "Bars", label.c_str(),
        "(%i) %s: %s/%s,   AI: %g",
        load_case,
        fmr::form::si_unit (perf.get_busy_data_speed (), "B   /s").c_str(),
        fmr::form::si_unit (perf.get_byte (), "B   ").c_str(),
        fmr::form::si_unit (this_time, "s").c_str(),
        double(perf.get_arithmetic_intensity ()));
    }

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);// Prototype (un)structured partition full FEA stack.
  const auto dofs_per_s = 150e6, base_time = 1e-6;
  //
  const auto targ_time = -1.0;// <=0.0 for no target time
  // Set mesh parameters. -----------------------------------------------------
  const fmr::Local_int do_elem_n = 9999;// 49999 about 30s, 9999 about 1s
  const bool do_rand_elem_n = false;// random problem size
  //
  constexpr bool is_free = true;// false for free, true overrides do_rand_len
  //
  const fmr::Phys_float bars_len  = 1.0;// 1m long
  bool do_rand_len = false;// perturb nodes.//TODO save coor to check.
  // Set physics parameters. --------------------------------------------------
  //
  const fmr::Enum_int load_case = 2;
  // 0: no load
  // 1: apply end displacement (only)
  // 2: apply end force (only),
  // 3: apply symmetric end displacements (exact at half iters)
  //    tip_u = end_load * bars_len / (bars_area * mtrl_modx)
  // 4: pin both ends, apply a displacement near the middle
  // 5: pin both ends, apply a force near the middle
  //
  const fmr::Phys_float end_disp = 0.001;// 1 mm
  const fmr::Phys_float end_load = 1e3;// 1 kN tension
  //
  const fmr::Phys_float bars_area = 0.010 * 0.010;// 1cm x 1cm cross section
  const fmr::Phys_float mtrl_modx = 200e9;// GPa (steel)
  //
  const fmr::Local_int elem_intp_n = 1;//TODO 0 for default?
  //
  // Set solver parameters. ---------------------------------------------------
  const fmr::Phys_float abs_tol = 1e-6;// absolute tolerance squared
  const fmr::Phys_float rel_tol = 1e-6;// relative tolerance squared
  //
  fmr::Local_int iter_max = 0;// set to zero for max (or target)
  // ==========================================================================
  const auto is_tile = !is_free;
  if (targ_time > 0.0) {iter_max = 0;}
  if (is_tile) {do_rand_len = false;}
  fmr::Local_int elem_mod;//ok for now, should pass tile instead
  if (is_tile){
    elem_mod = 1;// structured tile elements
  } else {// unstructured free elements
    elem_mod = std::numeric_limits<fmr::Local_int>::max();
  }
  if (do_rand_elem_n) {
    //
  }
  using femera::grid::tile::Tile;
  using femera::grid::tile::Bar1;
  //
  constexpr fmr::Dim_int part_d = 1;
  constexpr fmr::Dim_int part_elem_p = 1;//NOTE higher-order Bars don't work.
  // --------------------------------------------------------------------------
  std::valarray<fmr::Local_int> data_elem_dims(3);
  data_elem_dims[0] = do_elem_n; data_elem_dims[1] = 0; data_elem_dims[2] = 0;
  std::valarray<fmr::Geom_float> phys_size(3);
  phys_size[0] = bars_len;
  phys_size[1] = 0.0; phys_size[2] = 0.0;
  std::valarray<fmr::Geom_float> tile_size(3);//TODO remove?
  tile_size[0] = bars_len / data_elem_dims[0];
  tile_size[1] = 0.0; tile_size[2] = 0.0;
  // Get derived mesh partition information.
  constexpr Tile<Bar1, part_elem_p> tile;
  const auto part_elem_n = tile.get_tile_part_elem_n (&data_elem_dims[0]);
  const auto part_node_n = tile.get_tile_part_node_n (&data_elem_dims[0]);
  const auto elem_node_n = tile.get_tile_elem_node_n ();// Bars elements
  // Make the mesh.
  std::valarray<fmr::Local_int> data_node_conn
    (std::size_t(part_elem_n) * elem_node_n);
 // Set up iterations ---------------------------------------------------------
  //
//#ifdef DEBUG_PRINT
#if 0
  {// local scope
    bool ok = halo_node_n > 0;
    mini->data->send (fmr::log, ok?" ok ":" :( ", "halo", "node",
      "%u in part", halo_node_n);
  }
#endif
  // Initialize physics -------------------------------------------------------
  const fmr::Local_int mtrl_prop_n = 1;
  const std::valarray<fmr::Phys_float>
  mtrl_prop(fmr::Phys_float(mtrl_modx * bars_area), mtrl_prop_n);// N
  //
  const fmr::Local_int node_dofs_n = part_d;// elasticity only
  const fmr::Local_int part_dofs_n = part_node_n * node_dofs_n;
  auto data_f = std::valarray<fmr::Phys_float>(part_dofs_n);// Zeroed in iter.
  auto data_u = std::valarray<fmr::Phys_float>(fmr::Phys_float(0.0), part_dofs_n);
  //TODO init part_u to constant strain.
  //
  femera::grid::test::BClist part_disp_bc;
  femera::grid::test::BClist part_load_bc;
  {// Scope boundary condition maps. These sort and dedupe BCs.
    femera::grid::test::BCmap map_disp_bc;
    femera::grid::test::BCmap map_load_bc;
    //
    map_disp_bc[0] = fmr::Phys_float(0.0);// Fix left end.
    //
    switch (load_case) {
      case 0: break;
      case 1: map_disp_bc[part_dofs_n - 1] = fmr::Phys_float(end_disp);
      break;
      case 2: map_load_bc[part_dofs_n - 1] = fmr::Phys_float(end_load);
      break;
      case 3: {
        map_disp_bc[0              ] = fmr::Phys_float(-end_disp);//overwrite
        map_disp_bc[part_dofs_n - 1] = fmr::Phys_float( end_disp);
      break;
      }
      case 4: {
        map_disp_bc[part_dofs_n - 1] = fmr::Phys_float(0.0);
        map_disp_bc[part_dofs_n / 2] = fmr::Phys_float(end_disp);
      break;
      }
      case 5: {
        map_disp_bc[part_dofs_n - 1] = fmr::Phys_float(0.0);
        map_load_bc[part_dofs_n / 2] = fmr::Phys_float(end_load);
      break;
      }
      default: break;
    }
    // Copy maps to lists.
    for (const auto &p : map_disp_bc) {
      part_disp_bc.emplace_back(p.first, p.second);
      //part_disp_bc.emplace_back(BCitem<>(p.first, p.second));
    }
    for (const auto &p : map_load_bc) {
      part_load_bc.emplace_back(p.first, p.second);
    }
  }
  // These are kept for correctness check after solve.
  auto part_f = &data_f[0];
  auto part_u = &data_u[0];
  //femera::grid::test::set_bcs(&part_u[0], part_disp_bc);
  //femera::grid::test::sum_bcs(&part_f[0], part_load_bc);
  //
  // Conjugate gradient is exact when iterated to the number of unknowns.
  if (iter_max == 0) {
    iter_max = (part_dofs_n < std::numeric_limits<fmr::Local_int>::max() )
    ? fmr::Local_int(part_dofs_n) - fmr::Local_int(part_disp_bc.size()) + 1
    : std::numeric_limits<fmr::Local_int>::max() -1;
  }
  fmr::Local_int test_n = 1;
  if (targ_time > 0.0) {
    auto time_est = iter_max * part_dofs_n / dofs_per_s + base_time;
    if (time_est < targ_time) {
      test_n = static_cast<fmr::Local_int>
        (targ_time / time_est + 1);
    } else {
      iter_max = static_cast<fmr::Local_int>
        (targ_time / time_est * iter_max + 1);
    }
    mini->data->send(fmr::log, "Bars","perf","test",
      "%ux %u iter %0.2fs target", test_n, iter_max, targ_time);
   }
  fmr::Local_int bad_jac_n = 0;
  const auto abs_tol2 = abs_tol * abs_tol;
  const auto rel_tol2 = rel_tol * rel_tol;
  //
  fmr::Phys_float init_res2 = std::numeric_limits<fmr::Phys_float>::max();
  fmr::Phys_float iter_res2 = std::numeric_limits<fmr::Phys_float>::max();
  fmr::Phys_float iter_rel2 = std::numeric_limits<fmr::Phys_float>::max();
    //
  auto iter_digits = fmr::math::count_digits(iter_max);
  if (iter_digits < 4) { iter_digits = 4; }
  const auto iter_form_str = "%" + std::to_string(iter_digits) + "u";
  fmr::Local_int this_iter = 0;// Initialization counts as iter 0.
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
    if (false && is_tile) {
      std::valarray<fmr::Geom_float>
      part_node_coor((elem_mod + 1) * part_d);
      part_node_coor[0] = 0.0;
      part_node_coor[1] = bars_len / fmr::Phys_float(elem_mod);
      
      bad_jac_n += femera::grid::test::calc_bars_invt_dets<1>( &part_jacs_dets[0],
        &part_node_coor[0], &data_node_conn[0], part_elem_n % elem_mod, part_d);
    } else {// Scope node_coor
      // Get node coordinates (temporarily).
      std::valarray<fmr::Geom_float>
      part_node_coor(std::size_t(part_node_n) * part_d);
      tile.get_tile_part_mesh (// returns halo_elem_n
        &data_node_conn[0], &part_node_coor[0],//NOTE elems halo sorted, nodes not.
        &tile_size[0],// only [0] used for Bar1 tile
        &data_elem_dims[0],
        part_d
      );
      if(do_rand_len && part_node_n > 2) {
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
    std::valarray<fmr::Phys_float> data_intp_wgts(1.0, std::size_t(elem_intp_n));
    {
    bool ok = (this_iter < iter_max) && (bad_jac_n == 0)
      && (iter_res2 > abs_tol2) && (iter_rel2 > rel_tol2)
      && (iter_res2 > 0.0) && (abs_tol2 > 0.0);
    mini->data->send (fmr::log, ok?" ok ":" :( ", "GO! ", "size",
      "%4u dofs %4u node %4u elem (%4u bad/%4u Jacs)",
      part_dofs_n, part_node_n, part_elem_n,
      bad_jac_n, part_elem_n * elem_intp_n );
    }
    //
    // Diagonal preconditioner (already inverted & synced)
    auto data_Minv = std::valarray<float>(1.0, part_dofs_n);
    femera::grid::test::set_bc0(&data_Minv[0], part_disp_bc);
    //femera::grid::test::set_bc0(&data_Minv[0], part_load_bc);
    //
    // Calculate the total data size for the part.
    fmr::Global_int data_scal_size = 0;
    fmr::Global_int data_node_size = 0;// scales with node/dofs
    fmr::Global_int data_elem_size = 0;// scales with elem count
    fmr::Global_int data_read_size = 0;// main memory read size
    fmr::Global_int data_save_size = 0;// main memory write size
    fmr::Global_int data_byte_size = 0;// allocated
    //
    // data in and out
    data_scal_size += sizeof(this_iter);
    data_scal_size += sizeof(init_res2);
    data_scal_size += sizeof(iter_res2);
    data_scal_size += sizeof(iter_rel2);
    data_node_size += sizeof(data_u[0]) * part_node_n * node_dofs_n;
    data_read_size += sizeof(data_u[0]) * part_node_n * node_dofs_n;
    data_save_size += sizeof(data_u[0]) * part_node_n * node_dofs_n;
    data_byte_size += data_u.size() * sizeof (fmr::Phys_float);
    data_node_size += sizeof(data_f[0]) * part_node_n * node_dofs_n;
    data_read_size += sizeof(data_f[0]) * part_node_n * node_dofs_n;
    data_save_size += sizeof(data_f[0]) * part_node_n * node_dofs_n;
    data_byte_size += data_f.size() * sizeof (fmr::Phys_float);
    //
    // const data in
    data_scal_size += sizeof(tile) * 0;// static constexpr
    data_scal_size += sizeof(iter_max);
    data_scal_size += sizeof(abs_tol2);
    data_scal_size += sizeof(rel_tol2);
    data_scal_size += sizeof(mtrl_prop[0]) * 1;
    data_byte_size += mtrl_prop.size() * sizeof (fmr::Phys_float);
    data_scal_size += sizeof(part_dofs_n);
    data_scal_size += sizeof(part_load_bc);
    data_scal_size += sizeof(fmr::Phys_float) * tile.get_elem_shpg_size(elem_intp_n);
    data_scal_size += sizeof(data_intp_wgts[0]) * 1;
    data_byte_size += data_intp_wgts.size() * sizeof (fmr::Phys_float);
    if (!is_tile) {//unstructured Free
      data_node_size += sizeof(data_Minv[0]) * part_node_n * node_dofs_n;
      data_read_size += sizeof(data_Minv[0]) * part_node_n * node_dofs_n;
      data_elem_size += sizeof(part_jacs_dets[0]) * part_elem_n * 2;
      data_elem_size += sizeof(data_node_conn[0]) * part_elem_n
      * tile.get_tile_elem_node_n ();
      data_read_size += sizeof(part_jacs_dets[0]) * part_elem_n * 2;
      data_read_size += sizeof(data_node_conn[0]) * part_elem_n
      * tile.get_tile_elem_node_n ();
      data_byte_size += data_intp_wgts.size() * sizeof (fmr::Phys_float);
      data_byte_size += data_Minv.size() * sizeof (fmr::Phys_float);
      data_byte_size += part_jacs_dets.size() * sizeof (fmr::Phys_float);
      data_byte_size += data_node_conn.size() * sizeof (fmr::Phys_float);
    } else {//structured Tile
      data_scal_size += sizeof(fmr::Phys_float) * node_dofs_n;//Minv (promoted)
      data_scal_size += sizeof(part_jacs_dets[0]) * 2;
      data_scal_size += sizeof(data_node_conn[0]) * tile.get_tile_elem_node_n ();
    }
    //
    // internal data
    data_scal_size += sizeof(part_d);
    data_scal_size += sizeof(node_dofs_n);
    data_scal_size += sizeof(elem_intp_n);
    data_scal_size += sizeof(data_elem_dims[0]) * 1;
    // this_elem_n, halo_elem_n, halo_node_n, conn_n
    data_scal_size += sizeof(fmr::Local_int) * 4;
    data_scal_size += sizeof(fmr::Dim_int) * 1;// this_elem_d
    //
    data_byte_size += data_scal_size;//allocated
    //
    using namespace femera::grid::test;
    //
    auto phsh = MeshPhys<Bar1, part_elem_p, fmr::Phys_float> ();
    auto slvr = PCGsProto<fmr::Phys_float> ();
    //
    auto time_iter = femera::Work::Work_time_t ("dofs");
    for (fmr::Local_int test_i = 0; test_i < test_n; ++test_i) {// test repeats
      this_iter = 0;
      init_res2 = std::numeric_limits<fmr::Phys_float>::max();
      iter_res2 = init_res2;
      iter_rel2 = init_res2;
      data_u = 0.0;// data_f = 0.0;
      femera::grid::test::set_bcs(&part_u[0], part_disp_bc);
      //
      data_byte_size += phsh.init(node_dofs_n, mtrl_prop_n,
        &data_elem_dims[0], &phys_size[0], &mtrl_prop[0]);
      //
      // Iterate ================================================================
      while ((this_iter < iter_max)
      && (iter_res2 > abs_tol2) && (iter_rel2 > rel_tol2)) {// Init is this_iter 0.
        using namespace femera::grid::test;
        
        const auto this_dofs_n
        = tile.get_tile_part_node_n (&data_elem_dims[0]) * node_dofs_n;
        //
        time_iter.add_idle_time_now();
        iter_res2 = phsh.get_part_resp (&part_f[0],
          (this_iter == 0 ? &part_u[0] : slvr.ptr_p()), elem_mod);
        //if (init_res2 < abs_tol2) {++this_iter; break;}
        init_res2 = (this_iter == 0) ? iter_res2 : init_res2;
        //time_iter.add_busy_time_now();
        // sync part_f
        iter_res2 = (this_iter == 0)
        ? slvr.init (&part_f[0], &data_Minv[0], part_load_bc, this_dofs_n)
        : slvr.iter (&part_f[0], &part_u[0], &data_Minv[0], this_dofs_n);
        init_res2 = (this_iter == 0) ? iter_res2 : init_res2;
        iter_rel2 = iter_res2 / init_res2;
        time_iter.add_busy_time_now();
#if 0
        if (elem_i == halo_elem_n - 1) {//TODO Synchronize halo nodes.
          for (fmr::Local_int h = 0; h < tile.halo_node_n (); ++h)
  #ifdef DEBUG_PRINT
            const auto //TODO Move halo_node_list to init OR renumber nodes.
            n = Tile<Bar1, tile.get_elem_p ()>::get_halo_node(h, &data_elem_dims[0]);
            if (this_iter < 5) {
              if (this_iter == 0) {
                mini->data->send (fmr::log, "sync", "halo", "node",
                  "%4u f: %+0.2e u: %+0.2e (elem %u done)",
                  n, part_f[n * node_dofs_n], part_u[n * node_dofs_n], elem_i);
              } else {
                mini->data->send (fmr::log, "sync", "halo", "node",
                  "%4u f: %+0.2e p: %+0.2e (elem %u done)",
                  n, part_f[n * node_dofs_n], part_p[n * node_dofs_n], elem_i);
              }
          }
  #else
            Tile<Bar1, tile.get_elem_p ()>::get_halo_node(h, &part_elem_dims[0]);
          }
  #endif
#endif
      if (test_i < 2) {// verbose 1
        const auto digits = fmr::math::count_digits (fmr::Local_int(iter_max) * 2);
        fmr::Local_int mod = 1;
        for (fmr::Local_int i = 2; i < digits; ++i) { mod *= 10; }
        if ((this_iter < 5) || (this_iter % mod == 0)) {
          bool ok = (iter_res2 < abs_tol2) || (iter_rel2 < rel_tol2);
          const auto istr = this_iter == 0 ? "init" : "iter";
          if (this_iter == 0) {
            const auto format = iter_form_str + " /" + iter_form_str
            + " %.2e,%.2e %2s %.2e,%.2e rel,abs tol";
            mini->data->send (fmr::log, "Bar1", "PCGs", istr,
              format.c_str(), this_iter, iter_max-1,
              sqrt(iter_rel2), sqrt(iter_res2), ok?"< ":">=",
              sqrt(rel_tol2), sqrt(abs_tol2) );
          } else {
            const auto format = iter_form_str + " /" + iter_form_str + " %.2e";
            mini->data->send (fmr::log, "Bar1", "PCGs", istr,
              format.c_str(), this_iter, iter_max-1, sqrt(iter_rel2) );
          }
        }
      }
      ++this_iter;
    }//end iteration while loop
    time_iter.add_count(this_iter * part_dofs_n);
    time_iter.add_read(this_iter * data_read_size);
    time_iter.add_save(this_iter * data_save_size);
  }//end test_i loop
  --this_iter;// Count is off by one here.
  const auto this_idle = time_iter.get_idle_s();
  const auto this_busy = time_iter.get_busy_s();
  //
  const auto part_data_size = data_scal_size + data_node_size + data_elem_size;
  data_byte_size += slvr.get_byte_size ();
  mini->data->send (fmr::log, "Bar1", "PCGs", "data",
    "%s/%s: %s + %s + %s",
    fmr::form::si_unit (part_data_size, "B").c_str(),
    fmr::form::si_unit (data_byte_size, "B").c_str(),//allocated
    fmr::form::si_unit (data_scal_size, "B").c_str(),
    fmr::form::si_unit (data_node_size / part_node_n, "B/node").c_str(),
    fmr::form::si_unit (data_elem_size / part_elem_n, "B/elem").c_str()
  );
  {
    send_perf_results ("busy", load_case, time_iter, this_busy, this_iter, 1);
    //send_perf_results ("iter", load_case, time_iter, this_busy + this_idle,
    //  this_iter, 1);
  }
  //TODO move the (empty) PCGsProto constructor body to a .cpp when moved out of test namespace.
  //NOTE the PCGsProto slvr instance goes out of scope, calling inline defined
  //     destructor.
FMR_WARN_INLINE_OFF
  }// end scope part_jacs_dets
FMR_WARN_INLINE_ON
  {
  const auto format = iter_form_str + " /" + iter_form_str
  + " %.2e,%.2e %2s %.2e,%.2e rel,abs tol";
  bool ok = (iter_res2 < abs_tol2) || (iter_rel2 < rel_tol2);
  mini->data->send (fmr::log, ok?" ok ":" :( ", "DONE", "iter",
    format.c_str(), this_iter, iter_max-1,
    sqrt(iter_rel2), sqrt(iter_res2), ok?"< ":">=", sqrt(rel_tol2), sqrt(abs_tol2) );
  }
  //
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
    case 3: {// Equal and opposite end displacements.
      const auto chck = -end_disp + 2 * end_disp
        * (fmr::Phys_float(i) / fmr::Phys_float(part_node_n - 1));
      ok += std::abs(part_u[i] - chck) < numtol * end_disp;
      break;
    }
    case 4: {// 1 cm mid displacement
      ok = 9999;//TODO
      break;
    }
    case 5: {// 1 cm mid force
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