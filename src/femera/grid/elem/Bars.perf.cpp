#include "../../femera.hpp"
#include "Bars.hpp"
#include "../tile/Bar1.hpp"
#include "../../../fmr/fems/elem-jacs.hpp"
#include "../../../fmr/fems/elem-poly.hpp"
#include "../../../fmr/smat.hpp"

#include <valarray>

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
  fmr::Geom_float part_node_coor[part_node_n * part_d];
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
  for ( fmr::Local_int n=0; n < part_node_n; ++n) {
    fmr::Geom_float buf[4];
    const auto ok = tile.get_tile_part_node_coor
      (buf, n, part_elem_dims, tile_size);
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
 // Initialize solver ---------------------------------------------------------
 //
 // Set up iterations ---------------------------------------------------------
  using femera::grid::elem::Elem;
  constexpr Elem<femera::grid::elem::Bars, part_elem_p> elem;
  //
  // Double-check element node count.
  bool ok = elem.get_node_n () == elem_node_n;
  mini->data->send (fmr::log, ok?" ok ":" :( ", "elem", "node",
    "%u each",elem.get_node_n());
  //
  const auto halo_elem_n = tile.get_halo_elem_n (part_elem_dims);
  //
  const fmr::Local_int iter_max = part_node_n * part_d -2;
  const fmr::Phys_float tol2 = 1e-6;
  // Iterate ------------------------------------------------------------------
  fmr::Phys_float r2 = 1.0;
  fmr::Local_int iter = 0;
  while (iter < iter_max && r2 < tol2) {
    ++iter;
    // Do halo elements.
    for (fmr::Local_int elem_i = 0; elem_i < halo_elem_n; ++elem_i) {
      // do stuff
    }
    // Synchronize halo nodes.
    //
    // Do interior elements.
    for (fmr::Local_int elem_i = halo_elem_n; elem_i < part_elem_n; ++elem_i) {
      // do stuff
    }
    //
    r2 = 1.0;
  }// end iteration while loop
//
  return mini->exit ();
}
namespace femera { namespace test {

} }//end femera::test namespace

#undef DEBUG_PRINT