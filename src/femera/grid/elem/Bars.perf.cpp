#include "../../femera.hpp"
#include "Bars.hpp"
#include "../tile/Bar1.hpp"
#include "../../../fmr/fems/elem-jacs.hpp"
#include "../../../fmr/fems/elem-poly.hpp"
#include "../../../fmr/smat.hpp"

#include <type_traits>

//#include <valarray>

#include <gtest/gtest.h>

#define DEBUG_PRINT

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  // Prototype unstructured mesh partition full FEA stack.
  // Set mesh parameters. -----------------------------------------------------
  using femera::grid::tile::Tile;
  // Set mesh partition parameters.
  const fmr::Dim_int part_d = 1;
  const fmr::Dim_int part_elem_p = 1;
  const fmr::Local_int part_elem_dims[3] = {9, 1, 1};
        const fmr::Geom_float tile_size[3] = {1.0 / part_elem_dims[0], 0.0, 0.0};
  // Get derived mesh partition information.
  constexpr Tile<femera::grid::tile::Bar1, part_elem_p> tile;
  const auto part_elem_n = tile.get_tile_part_elem_n (part_elem_dims);
  const auto part_node_n = tile.get_tile_part_node_n (part_elem_dims);
  const auto elem_node_n = tile.get_tile_elem_node_n ();// Bars elements
  // Make the mesh.
  #if 0
  std::valarray<fmr::Local_int>  part_node_conn(part_elem_n * elem_node_n);
  std::valarray<fmr::Geom_float> part_node_coor(part_node_n * part_d);
  #else
  fmr::Local_int part_node_conn[part_elem_n * elem_node_n];
  #endif
  // Connectivity: first halo elements, then the remaining elements.
  for (fmr::Local_int el = 0; el < part_elem_n; ++el) {
    const auto h = tile.get_halo_sort_elem (el, part_elem_dims);
    //const auto el_i = el;
    const auto ok = tile.get_tile_node_conn (&part_node_conn[h * elem_node_n], h,
      part_elem_dims);
#ifdef DEBUG_PRINT
    if (el < 5) {
      const auto i = h * elem_node_n;
      mini->data->send (fmr::log, ok?" ok ":" :( ", "tile", "elem",
        "%u conn: %u %u (halo sort)",
        h, part_node_conn[i], part_node_conn[i+1]);
    }
#endif
  }
  // Set node coordinates.
  const fmr::Dim_int elem_d = tile.get_elem_d ();
  fmr::Phys_float part_jacs_dets[part_elem_n * (part_d*elem_d + 1)];
  {// Scope node_coor
    fmr::Geom_float part_node_coor[part_node_n * part_d];
    for ( fmr::Local_int n=0; n < part_node_n; ++n) {
      fmr::Geom_float buf[4];
      const auto ok = tile.get_tile_part_node_coor
        (buf, n, part_elem_dims, tile_size);//TODO FIXME get rid of need for buff.
      for (fmr::Dim_int d = 0; d < part_d; ++d) {
        part_node_coor[n * part_d + d] = buf[d];
      }
#ifdef DEBUG_PRINT
      if (n < 5) {
        const auto i = n * part_d;
        mini->data->send (fmr::log, ok?" ok ":" :( ", "tile", "node",
          "%u coor: %f", n, double(part_node_coor[i]));
      }
#endif
    }
    // Calc jacs_dets.
    for (fmr::Local_int el = 0; el < part_elem_n; ++el) {
      // Calculate Jacobian determinants for each element.
      // Placeholder calculation: set all to 1.0 for now.
      for (fmr::Dim_int d = 0; d < part_d * elem_d + 1; ++d) {
        part_jacs_dets[el * (part_d * elem_d + 1) + d] = 1.0;//TODO correct this.
      }
#ifdef DEBUG_PRINT
      if (el < 5) {
        const bool ok = (part_jacs_dets[el * (part_d * elem_d + 1)]
          - 1.0 / part_jacs_dets[el * (part_d * elem_d + 1) + 1])
          < std::numeric_limits<fmr::Phys_float>::epsilon();
        mini->data->send (fmr::log, ok?" ok ":" :( ", "jacd", "elem",
          "%u %f == 1.0 / %f", el, 
          double(part_jacs_dets[el * (part_d * elem_d + 1)]),
          double(part_jacs_dets[el * (part_d * elem_d + 1) + 1]));
      }
#endif
    }
  }// Toss part_node_coor. Keep part_jacs_dets.
 // Initialize solver ---------------------------------------------------------
 //
 // Set up iterations ---------------------------------------------------------
  using femera::grid::elem::Elem;
  constexpr Elem<femera::grid::elem::Bars, part_elem_p> elem;
  //
#ifdef DEBUG_PRINT
  // Double-check element dimension and node count.
  {// local scope
    bool ok = elem.get_elem_d () == elem_d;
    mini->data->send (fmr::log, ok?" ok ":" :( ", "elem", " dim",
      "%u", u_int(elem.get_elem_d()));
    //
    ok = elem.get_node_n () == elem_node_n;
    mini->data->send (fmr::log, ok?" ok ":" :( ", "elem", "node",
      "%u each", elem.get_node_n());
  }
#endif
  //
  const auto halo_elem_n = tile.get_halo_elem_n (part_elem_dims);
  const auto halo_node_n = tile.get_halo_node_n (part_elem_dims);
#ifdef DEBUG_PRINT
  {// local scope
    bool ok = halo_node_n > 0;
    mini->data->send (fmr::log, ok?" ok ":" :( ", "halo", "node",
      "%u in part", halo_node_n);
  }
#endif
  //
  const fmr::Local_int iter_max = part_node_n * part_d - 2;// number of unknowns
  const fmr::Phys_float tol2 = 1e-6;
  // Iterate ------------------------------------------------------------------
  fmr::Phys_float r2 = 0.0;// largest
  fmr::Local_int iter = 0;
  while (iter < iter_max && r2 < tol2) {
    ++iter;
    // Do halo elements.
    for (fmr::Local_int elem_i = 0; elem_i < halo_elem_n; ++elem_i) {
      // Do physics.
    }
    // Synchronize halo nodes.
    for (fmr::Local_int halo_node_i = 0; halo_node_i < halo_node_n; ++halo_node_i) {
      // Synchronize.
    }
    // Do interior elements.
    for (fmr::Local_int elem_i = halo_elem_n; elem_i < part_elem_n; ++elem_i) {
      // Do physics.
    }
    //
    r2 = std::numeric_limits<fmr::Phys_float>::max ();//TODO Ensure exit for now.
  }// end iteration while loop
//
  return mini->exit ();
}
namespace femera { namespace test {

} }//end femera::test namespace

#undef DEBUG_PRINT