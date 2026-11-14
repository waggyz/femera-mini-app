#ifndef FEMERA_HAS_GRID_ELEM_HPP
#define FEMERA_HAS_GRID_ELEM_HPP

#include "../../fmr/fmr.hpp"
#include "../../fmr/math.hpp"

namespace femera { namespace grid { namespace elem {//TODO grid -> geom
//namespace femera { namespace geom { //TODO change to here?
#if 0
// Forward-delare element shapes TODO is this needed?
class Pins;// 1D point element for "hard" pins & arbitrary point forces
class Bars;
class Ipin;// 1D interface element for "soft" pins (force-displacement relation)
class Ibar;// 1D interface element embedded in 2D or 3D space
class Itri; class Iqud;// 2D interface elements embedded in 3D space
class Tris; class Quad;
class Tets; class Prmd; class Wdge; class Cube;
//class User;//TODO user element
#endif
template <class T, typename fmr::Dim_int P=1u, typename F=fmr::Geom_float>
class Elem {//TODO consider pulling P out of the template parameters. 
  /*
  This class only defines serendipity polynomial elements for order P in [1,2,3].
  Other elements (e.g., Lagrangian) will be handleed seperately.

  For higher-order elements, edge and side nodes are assumed to 
  be placed at their natural positions along edges and faces. Node placement
  along curved edges and faces is not supported by this class.
  This puts an upper limit on the polynomial order needed to accurtely
  represent the element geometry. I.e., using higher-order integration than the
  default will not improve the already exact integration performed.

  Elements are defined in 3D by derived classes and arrays reduced as needed
  to match the simulation spatial dimension template parameter (D).

  Node numbering follows Gmsh canonical ordering.
  See: https://gmsh.info/dev/doc/texinfo/gmsh.html#Node-ordering
  NOTE: this code may or may not have the same prism/Wdge exception.
  TODO double-check serendipity elements for the use of face nodes.
  
  Members are constexpr so that everything is kept thread-local to the instance
  that created it.
  */
  public:
  // Simple constexpr accessors -----------------------------------------------
  // constexpr methods are implicitly inline
  static constexpr fmr::math::Poly get_poly_type () noexcept {
    return T::poly_type;}
  static constexpr fmr::Dim_int   get_elem_p () noexcept {return P;}
  //
  static constexpr fmr::Dim_int   get_elem_d () noexcept {return T::elem_d;}
  static constexpr fmr::Local_int get_vert_n () noexcept {return T::vert_n;}
  static constexpr fmr::Local_int get_edge_n () noexcept {return T::edge_n;}
  static constexpr fmr::Local_int get_tris_n () noexcept {return T::tris_n;}
  static constexpr fmr::Local_int get_quad_n () noexcept {return T::quad_n;}
  static constexpr fmr::Local_int get_vols_n () noexcept {return T::vols_n;}
  //
  static constexpr F get_edge_l () noexcept {return F(T::edge_l);}
  static constexpr F get_face_a () noexcept {return F(T::face_a);}
  static constexpr F get_elem_v () noexcept {return F(T::elem_v);}
  //
  // Non-trivial constexpr scalar member functions ----------------------------
  static constexpr fmr::Local_int get_face_n () noexcept;
  static constexpr fmr::Local_int get_node_n () noexcept;
  //
  // Array constexpr pointers =================================================
  // Natural element coordinates
  //NOTE these currently return 3D coordinates, regardless of element dimension.
  static constexpr bool get_elem_node_coor (F* node_coor,
    const F* vert_coor = T::template vert_coor<F> ) noexcept;
  //
  static constexpr const F* get_elem_vert_coor ()// vert major, dims minor
  noexcept { return T::template vert_coor<F>;// transposed coor_vert
  }
  static constexpr const F* get_elem_coor_vert ()// dims major, vert minor
  noexcept { return T::template coor_vert<F>;// (transposed vert_coor)
  }
  // Element constituent connectivity
  //NOTE get_vert_conn () is trivial = {0,1,...,get_vert_n()-1}
  //     get_node_conn () is trivial = {0,1,...,get_node_n()-1}
  //
  //NOTE the following are all only vertex connectivity (p==1) arrays.
  static constexpr const fmr::Local_int* get_bars_vert_conn () noexcept {
    return T::bars_vert_conn;}
  static constexpr const fmr::Local_int* get_tris_vert_conn () noexcept {
    return T::tris_vert_conn;}
  static constexpr const fmr::Local_int* get_quad_vert_conn () noexcept {
    return T::quad_vert_conn;}
  //
  // Element quality calculations ---------------------------------------------
#if 0
  static constexpr F get_edge_qual (
    const F* node_coor, const fmr::Dim_int dims=3) noexcept;
  static constexpr F get_skew_qual (
    const F* node_coor, const fmr::Dim_int dims=3) noexcept;
#endif
  static constexpr F get_delt_qual (
    const F* node_coor, const fmr::Dim_int dims=3, const bool is_squared=false)
  noexcept;
  //
  // Jacobian and polynomial sizes ============================================
  static constexpr
  fmr::Local_int jacs_size (
    const fmr::Local_int intp_n=1u, const fmr::Dim_int sims_d=3u)
  noexcept {return intp_n * (sims_d*sims_d);}
  static constexpr
  fmr::Local_int jacd_size (
    const fmr::Local_int intp_n=1, const fmr::Dim_int sims_d=3u)
  noexcept {return intp_n * (sims_d*sims_d + 1u);}
  static constexpr
  fmr::Local_int get_poly_func_size (const fmr::Local_int intp_n=1u) noexcept;
  static constexpr
  fmr::Local_int get_poly_grad_size (
    const fmr::Local_int intp_n=1, const fmr::Dim_int sims_d=3u) noexcept;
  //
};//end Elem

} } }//end femera::grid::elem:: namespace

#include "Elem.ipp"

//end FEMERA_HAS_GRID_ELEM_HPP
#endif
