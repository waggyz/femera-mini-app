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
  //TODO consider using P == 0 for non-serendipity elements.
  //TODO one more Elem template parameter for integration rule <R>?
  //TODO Consider enums for element type, order (# nodes), integration rule.
  //TODO Change to template <class T, Elem_args A, typename F?
  //     But, does this allow enable_if on the struct content? - YES
  //
  public:
  // Simple constexpr accessors -----------------------------------------------
  // constexpr methods are implicitly inline
  static constexpr fmr::math::Poly get_poly_type () noexcept {
    return T::poly_type;}
  static constexpr fmr::Dim_int get_elem_p () noexcept {return P;}
  //
  static constexpr fmr::Dim_int get_elem_d () noexcept {
    //static_assert( T::elem_d <= D, //NOTE only checked if this function used.
    //  "Element dimension cannot exceed simulation spatial dimension.");
    return T::elem_d;}
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
  static constexpr
  fmr::Local_int jacs_size (
    const fmr::Local_int intp_n=1u, const fmr::Dim_int sims_d=3u
  ) noexcept {return intp_n * (sims_d*sims_d);}
  static constexpr
  fmr::Local_int jacd_size (
    const fmr::Local_int intp_n=1, const fmr::Dim_int sims_d=3u
  ) noexcept {return intp_n * (sims_d*sims_d + 1u);}
  static constexpr
  fmr::Local_int get_poly_func_size (const fmr::Local_int intp_n=1u) noexcept;
  static constexpr
  fmr::Local_int get_poly_grad_size (
    const fmr::Local_int intp_n=1, const fmr::Dim_int sims_d=3u) noexcept;
  //
  // Array constexpr pointers =================================================
  //NOTE get_vert_conn () is trivial = {0,1,...,get_vert_n()-1}
  //     get_node_conn () is trivial = {0,1,...,get_node_n()-1}
  //
  //NOTE the following are all only vertex connectivity (p==1) arrays.
  static constexpr const fmr::Local_int* get_bars_conn () noexcept {
    return T::bars_conn;}
  static constexpr const fmr::Local_int* get_tris_conn () noexcept {
    return T::tris_conn;}
  static constexpr const fmr::Local_int* get_quad_conn () noexcept {
    return T::quad_conn;}
  //
  // Natural element coordinates ----------------------------------------------
  //NOTE these currently return 3D coordinates,
  //     regardless of element or simulation dimension.
  //
  static constexpr bool get_elem_node_coor (F* node_coor,
    const F* vert_coor = T::vert_coor ) noexcept;
  //
  //NOTE The following may not be the best way to dispatch.
  template <typename G=F,
    typename std::enable_if <std::is_same<G, double>::value>::type* = nullptr>
  static constexpr const G* get_elem_vert_coor () noexcept {
    return T::vert_coor;
  }
  template <typename G=F,
    typename std::enable_if <std::is_same<G, double>::value>::type* = nullptr>
  static constexpr const G* get_elem_coor_vert () noexcept {
    return T::coor_vert;
  }
  template <typename G=F,
    typename std::enable_if <std::is_same<G, float>::value>::type* = nullptr>
  static constexpr const G* get_elem_vert_coor () noexcept {
    return T::vert_coor_f;
  }
  template <typename G=F,
    typename std::enable_if <std::is_same< G,float>::value>::type* = nullptr>
  static constexpr const G* get_elem_coor_vert () noexcept {
    return T::coor_vert_f;
  }
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
};//end Elem

} } }//end femera::grid::elem:: namespace

#include "Elem.ipp"

//end FEMERA_HAS_GRID_ELEM_HPP
#endif
