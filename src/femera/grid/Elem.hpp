#ifndef FEMERA_HAS_GRID_ELEM_HPP
#define FEMERA_HAS_GRID_ELEM_HPP

#include "../../fmr/fmr.hpp"
#include "../../fmr/math.hpp"
//#include "../../fmr/smat.hpp"// needed in Elem.ipp for Jacobian inverse & det?

namespace femera { namespace grid { namespace elem {//TODO grid -> geom
//namespace femera { namespace geom { //TODO change to here?
#if 0
// Forward-delare element shapes TODO is this needed?
//class Pins;//TODO point element for "soft" pins & arbitrary point forces?
class Bars;
//class Ipts;//TODO 1D interface element embedded in 1D, 2D, 3D space
class Ibar;// 1D interface element embedded in 2D or 3D space
class Itri; class Iqud;// 2D interface elements embedded in 3D space
class Tris; class Quad;
class Tets; class Prmd; class Wdge; class Cube;
//class User;//TODO user element
#endif
template <class T, typename fmr::Local_int P=1, typename F=fmr::Geom_float>
class Elem {
  /*
  Elements are defined in 3D by derived classes and arrays reduced as needed
  to match the simulation spatial dimension template parameter (D).

  These only define serendipity polynomial elements for order P in [1,2,3].
  Other elements (e.g., Lagrangian) will be handleed seperately.

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
  //TODO Change to template <class T, Elem_args A, typename F>, as above?
  //     But, does this allow enable_if on the struct content? - YES
  //TODO memalign member variables
#if 0
  static_assert (D >= T::elem_d,//TODO move to constructor?
  "Element can only be embedded in a "
  "simulation of equal or greater spatial dimension.");
#endif
  //
#if 0
// Should this class remain stateless? //TODO Move to FEms?
  private:
  // Element quality criteria: warn if bad.
  F dets_qual_min = F(0.2);// min (det) / abs (max (det)) at vert_coor
  // bars,tris,tets: const det > std::numeric_limits<F>::min() [* part_size??]
  //                          OR std::numeric_limits<F>::epsilon()
  //NOTE these are all vert_n == elem_d + 1 (when side nodes at natural positions)
  F edge_qual_min = F(0.2);// shortest / longest edge
  F skew_qual_min = F(0.6);// = 1.0-skewness [0-1] (quad, cube)
  F delt_qual_min// ratio of min edge length / max coord absolute value
  = F(std::numeric_limits<F>::epsilon ());
  //
  public:
  // Simple get / set / compare ----------------------------------------------
  //
  F get_dets_tol () const noexcept {return dets_qual_min;}
  F get_edge_tol () const noexcept {return edge_qual_min;}
  F get_skew_tol () const noexcept {return skew_qual_min;}
 //
  F set_dets_tol (const F val) noexcept {return dets_qual_min = F(val);}
  F set_edge_tol (const F val) noexcept {return edge_qual_min = F(val);}
  F set_skew_tol (const F val) noexcept {return skew_qual_min = F(val);}
  //
  bool is_dets_qual_ok (const F val) const noexcept {return val >= dets_qual_min;}
  bool is_edge_qual_ok (const F val) const noexcept {return val >= edge_qual_min;}
  bool is_skew_qual_ok (const F val) const noexcept {return val >= skew_qual_min;}
#endif
  //
  public:
  // Simple constexpr accessors -----------------------------------------------
  // constexpr functions are implicitly inline
  constexpr fmr::Local_int get_elem_p () const noexcept {return P;}
  //constexpr fmr::Local_int get_sims_d () const noexcept {return D;}
  constexpr fmr::Local_int get_sims_d () const noexcept {return T::sims_d;}
  constexpr fmr::Local_int get_elem_d () const noexcept {
    //static_assert( T::elem_d <= D, //NOTE only checked if this function used.
    //  "Element dimension cannot exceed simulation spatial dimension.");
    return T::elem_d;}
  constexpr fmr::Local_int get_vert_n () const noexcept {return T::vert_n;}
  constexpr fmr::Local_int get_edge_n () const noexcept {return T::edge_n;}
  constexpr fmr::Local_int get_tris_n () const noexcept {return T::tris_n;}
  constexpr fmr::Local_int get_quad_n () const noexcept {return T::quad_n;}
  constexpr fmr::Local_int get_vols_n () const noexcept {return T::vols_n;}
  //
  constexpr F get_edge_l () const noexcept {return F(T::edge_l);}
  constexpr F get_face_a () const noexcept {return F(T::face_a);}
  constexpr F get_elem_v () const noexcept {return F(T::elem_v);}
  //
  constexpr fmr::math::Poly get_poly_type () const noexcept {return T::poly_type;}
//
  // Non-trivial constexpr member functions -----------------------------------
  constexpr fmr::Local_int get_face_n () const noexcept;
  constexpr fmr::Local_int get_node_n () const noexcept;
  constexpr fmr::Local_int get_tile_node_n () const noexcept;
  //
  constexpr
  fmr::Local_int jacs_size (
    const fmr::Local_int intp_n=1, const fmr::Local_int sims_d=3
  ) const noexcept;
  constexpr
  fmr::Local_int jacd_size (
    const fmr::Local_int intp_n=1, const fmr::Local_int sims_d=3
  ) const noexcept;
  constexpr
  fmr::Local_int poly_func_size (const fmr::Local_int intp_n=1) const noexcept;
  constexpr
  fmr::Local_int poly_grad_size (
    const fmr::Local_int intp_n=1, const fmr::Local_int sims_d=3) const noexcept;
  //
  // Array constexpr pointers =================================================
  //
#if 0
  // vert_conn and node_conn are trivial: just consecutive integers.
  constexpr const fmr::Local_int* get_vert_conn () const noexcept {
    return T::vert_conn;}
#endif
  //NOTE the following are all only vertex connectivity (p==1) arrays.
  constexpr const fmr::Local_int* get_bars_conn () const noexcept {
    return T::bars_conn;}
  constexpr const fmr::Local_int* get_tris_conn () const noexcept {
    return T::tris_conn;}
  constexpr const fmr::Local_int* get_quad_conn () const noexcept {
    return T::quad_conn;}
  //
  // Natural element vertex coordinates --------------------------------------
  //NOTE these are only valid when element dimension matches simulation dimension.
  template <typename Ft=F,
    typename std::enable_if <std::is_same<Ft, double>::value>::type* = nullptr>
  constexpr const F* get_vert_coor () const noexcept {
    return T::vert_coor;
  }
  template <typename Ft=F,
    typename std::enable_if <std::is_same<Ft, double>::value>::type* = nullptr>
  constexpr const F* get_coor_vert () const noexcept {
    return T::coor_vert;
  }
  template <typename Ft=F,
    typename std::enable_if <std::is_same<Ft, float>::value>::type* = nullptr>
  constexpr const F* get_vert_coor () const noexcept {
    return T::vert_coor_f;
  }
  template <typename Ft=F,
    typename std::enable_if <std::is_same< Ft,float>::value>::type* = nullptr>
  constexpr const F* get_coor_vert () const noexcept {
    return T::coor_vert_f;
  }
  // Structured grid element tiles ----------------------------------------
  constexpr fmr::Local_int get_tile_n () const noexcept {
    return T::tile_elem_n;}
  constexpr fmr::Local_int get_tile_vert_n () const noexcept {
    return T::tile_vert_n;}
  constexpr fmr::Local_int get_tile_edge_n () const noexcept {
    return T::tile_edge_n;}
  constexpr fmr::Local_int get_tile_face_n () const noexcept {
    return T::tile_tris_n;}
  constexpr fmr::Local_int get_tile_quads_n () const noexcept {
    return T::tile_quads_n;}
  constexpr fmr::Local_int get_tile_vols_n () const noexcept {
    return T::tile_vols_n;}// 3D elements of this type (assume all same)
  //
  constexpr const fmr::Local_int* get_tile_vert_conn () const noexcept {
    return T::tile_vert_conn;}
  //
  //TODO how to handle when grid_size not evenly disible by part_size?
  constexpr bool get_tile_vert_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,
    const fmr::Local_int* part_size)// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    //NOTE conn, part_size bounds should be checked by the caller.
    noexcept;
  constexpr bool get_tile_vert_conn_prdc (fmr::Local_int* conn,// Periodic BCs
    //NOTE This is for a periodic partion, not an overall periodic grid.
    const fmr::Local_int tile_i,
    const fmr::Local_int* part_size)// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    //NOTE conn, part_size bounds should be checked by the caller.
    noexcept;
  //
  constexpr bool get_tile_vert_coor(fmr::Geom_float* vert_coor,
    const fmr::Local_int node_i,
    const fmr::Local_int node_n,
    const fmr::Geom_float* elem_size,
    const fmr::Geom_float* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE node_coor, elem_size, part_orig bounds should be checked by the caller.
    noexcept;
  constexpr bool get_tile_node_coor(fmr::Geom_float* node_coor,
    const fmr::Local_int node_i,
    const fmr::Local_int node_n,
    const fmr::Geom_float* elem_size,
    const fmr::Geom_float* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE node_coor, elem_size, part_orig bounds should be checked by the caller.
    noexcept;

  constexpr bool get_tile_elem_vert_coor(//TODO need: 
    fmr::Geom_float* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i,
    const fmr::Local_int elem_n,
    const fmr::Geom_float* elem_size,
    const fmr::Geom_float* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE vert_coor, elem_size, part_orig bounds should be checked by the caller.
    noexcept;
  constexpr bool get_tile_elem_coor_vert(
    fmr::Geom_float* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i,
    const fmr::Local_int elem_n,
    const fmr::Geom_float* elem_size,
    const fmr::Geom_float* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE vert_coor, elem_size, part_orig bounds should be checked by the caller.
    noexcept;
  //
#if 0
  template <typename Y=F>
  Y* jatd_naos (const fmr::Local_int intp_n=1,// inverted and transposed
    // Jacobian & determinants
    const fmr::Dim_int dims=D, fmr::Local_int node_n=get_node_n (),
    const F* node_coor) const noexcept;
  
  template <typename Y=F>
  Y* jacs_csoa (const fmr::Local_int intp_n=1,// x1,x2,..., y1,y2,..., z1,z2,...
    const fmr::Dim_int dims=D, fmr::Local_int node_n=get_node_n (),
  const F* coor_node) const noexcept;
  template <typename Y=F>
  Y* jatd_nsoa (const fmr::Local_int intp_n=1,
    const fmr::Dim_int dims=D, fmr::Local_int node_n=get_node_n (),
  const F* coor_node[D*get_node_n ()]) const noexcept;
#endif
  //
  // Element quality calculations
#if 0
  F calc_dets_qual (const F* node_coor, const fmr::Dim_int dims=3) const noexcept;
  F calc_edge_qual (const F* node_coor, const fmr::Dim_int dims=3) const noexcept;
  F calc_skew_qual (const F* node_coor, const fmr::Dim_int dims=3) const noexcept;
#endif
  constexpr
  F calc_delt_qual (const F* node_coor, const fmr::Dim_int dims=3) const noexcept;
  //
  // Element static functions (independent of elem type) =====================
  //
  // =========================================================================
#if 0
  constexpr
  F face_norm [sims_d * face_n]]//TODO this can be evaluated at compile time.
  //NOTE specialize for simulation dimension <D>
    = {
  };
#endif
  //
#if 0
  template <fmr::Local_int tP=P,
    typename std::enable_if <tP == 1>::value>// P=1, D does not matter
  constexpr const fmr::Local_int* get_node_conn () noexcept {//TODO Needed?
  // This is just a list of integers [1..node_n]
    return T::vert_conn;
  }
#endif
  // Node coordinates --------------------------------------------------------
  //TODO Generalize for different element shapes by moving to Tets, etc.?
#if 0
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 1>::value,
    typename std::enable_if <tD == 3>::value>// P=1, D=3
  constexpr const F* get_node_coor () const noexcept {
   return T::vert_coor;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 1>::value,
    typename std::enable_if <tD == 3>::value>// P=1, D=3
  constexpr const F* get_coor_node () const noexcept {
    return T::coor_vert;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 2>::value,
    typename std::enable_if <tD == 3>::value>// P=2, D=3
  constexpr const F* get_node_coor () const noexcept {
   return T::tet10_coor;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 2>::value,
    typename std::enable_if <tD == 3>::value>// P=2, D=3
  constexpr const F* get_coor_node () const noexcept {
    return  T::coor_tet10;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 3>::value,
    typename std::enable_if <tD == 3>::value>// P=3, D=3
  constexpr const F* get_node_coor () const noexcept {
   return T::tet20_coor;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 3>::value,
    typename std::enable_if <tD == 3>::value>// P=3, D=3
  constexpr const F* get_coor_node () const noexcept {
    return  T::coor_tet20;
  }
#endif
  // Integration points and weights (default) --------------------------------
  //
  // Shape functions and gradients -------------------------------------------
  //
};

} } }//end femera::grid::elem:: namespace

#include "Elem.ipp"

//end FEMERA_HAS_GRID_ELEM_HPP
#endif
