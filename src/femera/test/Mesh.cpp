#include "Mesh.hpp"

#include "../grid/elem/Tets.hpp"

namespace femera { namespace test {
  Mesh reference_tet4 () {
    Mesh mesh;
    using Tet = femera::grid::elem::Tets;
    mesh.spatial_dimension = Tet::sims_d;
    mesh.node_count = Tet::vert_n;
    mesh.element_count = 1;
    mesh.nodes_per_element = Tet::vert_n;
    mesh.coordinates.assign (Tet::vert_coor,
      Tet::vert_coor + Tet::sims_d * Tet::vert_n);
    mesh.connectivity.assign (Tet::vert_conn,
      Tet::vert_conn + Tet::vert_n);
    mesh.boundary_nodes = {0, 1, 2, 3};
    return mesh;
  }

  std::vector<fmr::Geom_float>
  Mesh::component_major_coordinates () const {
    std::vector<fmr::Geom_float> converted;
    converted.reserve (coordinates.size ());
    for (fmr::Local_int dimension = 0;
         dimension < spatial_dimension; ++dimension) {
      for (fmr::Local_int node = 0; node < node_count; ++node) {
        converted.push_back (
          coordinates [node * spatial_dimension + dimension]);
      }
    }
    return converted;
  }
} }//end femera::test:: namespace
