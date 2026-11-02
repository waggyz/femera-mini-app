#ifndef FEMERA_HAS_TEST_MESH_HPP
#define FEMERA_HAS_TEST_MESH_HPP

#include "../../fmr/fmr.hpp"

#include <vector>

namespace femera { namespace test {
  struct Mesh {
    Mesh ()
    : spatial_dimension (0), node_count (0), element_count (0),
      nodes_per_element (0), coordinates (), connectivity (),
      boundary_nodes (), interior_nodes () {}

    fmr::Local_int spatial_dimension = 0;
    fmr::Local_int node_count = 0;
    fmr::Local_int element_count = 0;
    fmr::Local_int nodes_per_element = 0;
    std::vector<fmr::Geom_float> coordinates;// node-major
    std::vector<fmr::Local_int> connectivity;// element-major, zero-based
    std::vector<fmr::Local_int> boundary_nodes;
    std::vector<fmr::Local_int> interior_nodes;

    std::vector<fmr::Geom_float> component_major_coordinates () const;
  };

  Mesh reference_tet4 ();
} }//end femera::test:: namespace

#endif
