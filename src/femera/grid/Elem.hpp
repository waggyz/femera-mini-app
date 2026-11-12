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

  //TODO decide on the implementation of reduced integration elements.
  /*
  This class only defines serendipity polynomial elements for order P in [1,2,3].
  Other elements (e.g., Lagrangian) will be handleed seperately.

  For higher-order elements, edge and side nodes are assumed to 
  be placed at their natural positions along edges and faces. Node placement
  along curved edges and faces is not supported by this class.
  This puts an upper limit on the polynomial order needed to accurtely
  represent the element geometry. I.e., using higher-order integration than the
  default will not improve the already exact integration performed.

  All of the elements defined by this class can tile a cartesian grid of the
  same dimension as the element dimesion, even interface elements.
  First-order (P1) element tiled grids are compatible with other regular
  cartesian grids, e.g., for finite difference (FD analysis).
  Higher-order element tiled grids have "holes" that make them incompatible
  with regular cartesian grids.

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
  // Non-trivial constexpr scalar member functions ----------------------------
  constexpr fmr::Local_int get_face_n () const noexcept;
  constexpr fmr::Local_int get_node_n () const noexcept;
  //
  constexpr fmr::math::Poly get_poly_type () const noexcept {return T::poly_type;}
  constexpr
  fmr::Local_int jacs_size (
    const fmr::Local_int intp_n=1, const fmr::Local_int sims_d=3
  ) const noexcept;
  constexpr
  fmr::Local_int jacd_size (
    const fmr::Local_int intp_n=1, const fmr::Local_int sims_d=3
  ) const noexcept;
  constexpr
  fmr::Local_int get_poly_func_size (const fmr::Local_int intp_n=1) const noexcept;
  constexpr
  fmr::Local_int get_poly_grad_size (
    const fmr::Local_int intp_n=1, const fmr::Local_int sims_d=3) const noexcept;
  //
  // Array constexpr pointers =================================================
  //NOTE get_vert_conn () is trivial = {0,1,...,get_vert_n()-1}
  //     get_node_conn () is trivial = {0,1,...,get_node_n()-1}
  //
  //NOTE the following are all only vertex connectivity (p==1) arrays.
  constexpr const fmr::Local_int* get_bars_conn () const noexcept {
    return T::bars_conn;}
  constexpr const fmr::Local_int* get_tris_conn () const noexcept {
    return T::tris_conn;}
  constexpr const fmr::Local_int* get_quad_conn () const noexcept {
    return T::quad_conn;}
  //
  // Natural element coordinates ----------------------------------------------
  //NOTE these currently return 3D coordinates,
  //     regardless of element or simulation dimension.
  //
  constexpr bool get_elem_node_coor (F* node_coor,
    const F* vert_coor = T::vert_coor ) const noexcept;
  //
  //NOTE The following may not be the best way to dispatch.
  template <typename G=F,
    typename std::enable_if <std::is_same<G, double>::value>::type* = nullptr>
  constexpr const G* get_elem_vert_coor () const noexcept {
    return T::vert_coor;
  }
  template <typename G=F,
    typename std::enable_if <std::is_same<G, double>::value>::type* = nullptr>
  constexpr const G* get_elem_coor_vert () const noexcept {
    return T::coor_vert;
  }
  template <typename G=F,
    typename std::enable_if <std::is_same<G, float>::value>::type* = nullptr>
  constexpr const G* get_elem_vert_coor () const noexcept {
    return T::vert_coor_f;
  }
  template <typename G=F,
    typename std::enable_if <std::is_same< G,float>::value>::type* = nullptr>
  constexpr const G* get_elem_coor_vert () const noexcept {
    return T::coor_vert_f;
  }
  // Element quality calculations ---------------------------------------------
#if 0
  F get_edge_qual (const F* node_coor, const fmr::Dim_int dims=3) const noexcept;
  F get_skew_qual (const F* node_coor, const fmr::Dim_int dims=3) const noexcept;
#endif
  constexpr
  F get_delt_qual (const F* node_coor, const fmr::Dim_int dims=3) const noexcept;
  //
  // Jacobian and determinant calculations ------------------------------------
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
  // Structured grid element tiling ===========================================
  //
  constexpr fmr::Local_int get_tile_elem_n () const noexcept {
    return T::tile_elem_n;}
  constexpr fmr::Local_int get_tile_vert_n () const noexcept {
    return T::tile_vert_n;}
  constexpr fmr::Local_int get_tile_edge_n () const noexcept {
    return T::tile_edge_n;}
  constexpr fmr::Local_int get_tile_face_n () const noexcept {
    return T::tile_tris_n;}
  constexpr fmr::Local_int get_tile_quad_n () const noexcept {
    return T::tile_quad_n;}
  constexpr fmr::Local_int get_tile_vols_n () const noexcept {
    return T::tile_vols_n;}// 3D elements of this type (assume all same)
  //
  constexpr fmr::Local_int get_tile_elem_node_n () const noexcept;

  constexpr fmr::Local_int get_tile_part_elem_n (
    const fmr::Local_int* part_elem_dims)// size 3 for 1D,2D,3D elements
  const noexcept{
  //NOTE only [0] is used for 1D elements like Bars,
  //     for 2D elements only [0] and [1] are used,
  //     for 3D elements all three are used.
  //
    return T::tile_elem_n
    * ((part_elem_dims[0] > 0 && T::elem_d > 0) ? part_elem_dims[0] : 0)
    * ((part_elem_dims[1] > 0 && T::elem_d > 1) ? part_elem_dims[1] : 1)
    * ((part_elem_dims[2] > 0 && T::elem_d > 2) ? part_elem_dims[2] : 1);
  }
  constexpr fmr::Local_int get_tile_part_node_n (const fmr::Local_int* part_elem_dims)
  const noexcept{
    return T::calc_tile_part_node_n(P, part_elem_dims);
  }
  // Tiled (strucured mesh) node coordinates
  constexpr bool get_tile_part_node_coor (F* coor,
    const fmr::Local_int node_i,
    const fmr::Local_int* part_elem_dims,// partition dimensions nodes_x,node_y,node_z for 1D,2D,3D elems
    const F* tile_size,
    const F* part_orig = nullptr// default is {0.0,0.0,0.0}
  ) const noexcept{
    return T::calc_tile_part_node_coor (
      coor, node_i, P, part_elem_dims, tile_size, part_orig);
  }
  //NOTE node_coor, part_node_dims, tile_size, part_orig bounds checked by caller.
  // Orientation of the tile is specified by tile_dirs,
  // which changes where mesh "holes" are when tiling by, e.g., Wdge elements.
  //
  // P1 FE meshes are the same as FD grids, regardless of element type.
#if 0
  constexpr bool get_tile_elem_coor_node (F* node_coor,
    const fmr::Local_int elem_i);
#endif
#if 0
  constexpr bool get_tile_elem_node_coor(fmr::Geom_float* node_coor,
    const fmr::Local_int node_i,
    const fmr::Local_int node_n,
    const fmr::Geom_float* tile_size,
    const fmr::Geom_float* part_orig = {0.0,0.0,0.0})// origin should default to {0.0,0.0,0.0}
  //NOTE node_coor, part_orig bounds should be checked by the caller.
  //TODO consistent and periodically alternating orientations .
    const noexcept;
#endif
  // Tile operations dispatch to T's corresponding static calc_* method.
  // Shapes without an implementation fail at compile time when called.
  // Connectivity implementations receive p (defaulting to P), allowing
  // shape-specific behavior for each order without templating the shape.
  //TODO how to handle when grid_size not evenly divisible by part_size?
  constexpr bool get_tile_vert_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,
    const fmr::Local_int* part_size,
    const fmr::Local_int p=P)// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    //NOTE conn, part_size bounds should be checked by the caller.
    const noexcept{
      return T::calc_tile_vert_conn (conn, tile_i, part_size, p);
    }
  constexpr bool get_tile_vert_conn_prdc (fmr::Local_int* conn,// Periodic BCs
    //NOTE This is for a periodic partition, not an overall periodic grid.
    const fmr::Local_int tile_i,
    const fmr::Local_int* part_size,
    const fmr::Local_int p=P)// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    //NOTE conn, part_size bounds should be checked by the caller.
    const noexcept{
      return T::calc_tile_vert_conn_prdc (conn, tile_i, part_size, p);
    }
  //
  constexpr bool get_tile_node_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,
    const fmr::Local_int* part_size,
    const fmr::Local_int p=P)// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    //NOTE conn, part_size bounds should be checked by the caller.
    const noexcept{
      return T::calc_tile_node_conn (conn, tile_i, part_size, p);
    }
  //
  constexpr bool get_tile_vert_coor(F* vert_coor,
    const fmr::Local_int vert_i, const fmr::Local_int vert_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE node_coor, elem_size, part_orig bounds should be checked by the caller.
    const noexcept{
      return T::calc_tile_vert_coor (vert_coor, vert_i, vert_n, tile_size, part_orig);
    }
  //
  constexpr bool get_tile_elem_vert_coor(//TODO need: 
    F* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE vert_coor, elem_size, part_orig bounds should be checked by the caller.
    const noexcept{
      return T::calc_tile_elem_vert_coor (vert_coor, elem_i, elem_n, tile_size, part_orig);
    }
  constexpr bool get_tile_elem_coor_vert(
    F* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE vert_coor, tile_size, part_orig bounds should be checked by the caller.
    const noexcept{
      return T::calc_tile_elem_coor_vert (vert_coor, elem_i, elem_n, tile_size, part_orig);
    }
};//end Elem

} } }//end femera::grid::elem:: namespace

#include "Elem.ipp"

//end FEMERA_HAS_GRID_ELEM_HPP
#endif
