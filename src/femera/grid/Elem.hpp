#ifndef FEMERA_HAS_GRID_ELEM_HPP
#define FEMERA_HAS_GRID_ELEM_HPP

#include "../../fmr/fmr.hpp"
#include "../../fmr/smat.hpp"// needed in Elem.ipp for Jacobian inverse & det

namespace femera { namespace grid { namespace elem {//TODO grid -> geom
//namespace femera { namespace grid { //TODO move up to here?
/*
// Forward-delare element shapes TODO is this needed?
class Bars;
class Line;//TODO 1D interface element embedded in 2D or 3D space
class Itri; class Iqud;// 2D interface elements embedded in 3D space
class Tris; class Quad;
class Tets; class Prmd; class Wdge; class Cube;
class User;//TODO user element
*/
template <class T, typename fmr::Local_int P=1, typename fmr::Local_int D=3,
  typename F=fmr::Geom_float>
class Elem {
  /*
  Elements are defined in 3D by derived classes and arrays reduced as needed
  to match the simulation spatial dimension template parameter (D).
  
  Members are constexpr so that everything is kept thread-local to the instance
  that created it.
  */
  //FIXME higher-order elements are identified by node_n <N>, not elem_p <P>
  //      because there are different elements (e.g., Lagrangian, serendipity)
  //      with the same shape and order but a different number of nodes.
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
  F edge_qual_min = F(0.2);// shortest / longest shortest edge
  F skew_qual_min = F(0.6);// = 1.0-skewness [0-1] (quad, cube)
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
  constexpr fmr::Local_int get_sims_d () const noexcept {return D;}
  constexpr fmr::Local_int get_elem_d () const noexcept {
    static_assert( T::elem_d <= D, //NOTE only checked if this function used.
      "Element dimension cannot exceed simulation spatial dimension.");
    return T::elem_d;}
  constexpr fmr::Local_int get_vert_n () const noexcept {return T::vert_n;}
  constexpr fmr::Local_int get_vols_n () const noexcept {return T::vols_n;}
  constexpr fmr::Local_int get_edge_n () const noexcept {return T::edge_n;}
  constexpr fmr::Local_int get_tris_n () const noexcept {return T::tris_n;}
  constexpr fmr::Local_int get_quad_n () const noexcept {return T::quad_n;}
  //
  constexpr F get_edge_l () const noexcept {return F(T::edge_l);}
  constexpr F get_face_a () const noexcept {return F(T::face_a);}
  constexpr F get_elem_v () const noexcept {return F(T::elem_v);}
//
  // Non-trivial constexpr member functions -----------------------------------
  constexpr
  fmr::Local_int get_face_n () const noexcept;
  constexpr
  fmr::Local_int get_node_n () const noexcept;
  constexpr
  fmr::Local_int jacd_size (const fmr::Local_int intp_n=1) const noexcept;
  constexpr
  fmr::Local_int poly_func_size (const fmr::Local_int intp_n=1) const noexcept;
  constexpr
  fmr::Local_int poly_grad_size (const fmr::Local_int intp_n=1) const noexcept;
  //
  // Array constexpr pointers =================================================
  //
  //NOTE the following are all only vertex connectivity (p==1) arrays.
  constexpr const fmr::Local_int* get_bars_conn () const noexcept {
    return T::bars_conn;}
  constexpr const fmr::Local_int* get_tris_conn () const noexcept {
    return T::tris_conn;}
  constexpr const fmr::Local_int* get_quad_conn () const noexcept {
    return T::quad_conn;}
  constexpr const fmr::Local_int* get_vert_conn () const noexcept {
    return T::vert_conn;}
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
  //
  //TODO structured grid element tiles ----------------------------------------
  constexpr fmr::Local_int get_tile_elem_n () const noexcept {
    return T::tile_elem_n;}
  constexpr fmr::Local_int get_tile_node_n () const noexcept {
    return T::tile_node_n;}
  constexpr const fmr::Local_int* get_tile_conn () const noexcept {
    return T::tile_conn;}
  //
  constexpr bool get_tile_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,
    const fmr::Local_int* grid_size)// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    //NOTE conn, grid_size bounds should be checked by the caller.
    noexcept;
  constexpr bool get_tile_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,
    const fmr::Local_int* grid_size,// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    const fmr::Local_int* part_size)
    //NOTE conn, grid_size, part_size bounds should be checked by the caller.
  noexcept;
  //
  // Element functions ========================================================
  //F calc_bbox_size // 1D:length, 2D:area, 3D:volume
  //  (const F* node_coor, const fmr::Dim_int dims=D) noexcept;// needs node_n
  //
  // Element quality calculations
  F calc_dets_qual (const F* node_coor, const fmr::Dim_int dims=D) noexcept;
  F calc_edge_qual (const F* node_coor, const fmr::Dim_int dims=D) noexcept;
  F calc_skew_qual (const F* node_coor, const fmr::Dim_int dims=D) noexcept;
   
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
  //TODO Generalize for different element shapes by moving to Tets, etc.
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
