#ifndef FEMERA_HAS_GRID_ELEM_HPP
#define FEMERA_HAS_GRID_ELEM_HPP

#include "../../fmr/fmr.hpp"
#include "../../fmr/math.hpp"
#include "../../fmr/fems/elem-poly.hpp"
#include <cstddef>
#include <stdexcept>
#include <type_traits>

namespace femera { namespace grid { namespace elem {//TODO grid -> geom
//namespace femera { namespace geom { }}//TODO change to here?
#if 0
// Forward-delare element shapes TODO is this needed?
class Pins;// 1D point element for "soft" pins and point forces & displacements
class Bars;
class Ipin;// 1D interface element for springs (force-displacement relation)
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
  Other elements (e.g., Lagrangian) will be handled seperately.

  For higher-order elements, edge and side nodes are assumed to 
  be placed at their natural positions along edges and faces. Node placement
  along curved edges and faces is not supported by this class.
  This puts an upper limit on the polynomial order needed to accurtely
  represent the element geometry. I.e., using higher-order integration than the
  default will not improve the already exact integration performed.

  Reference dimensions are defined by derived topologies. Physical coordinate
  dimensions are supplied by the geometry mapping and need not equal them;
  canonical reference-coordinate arrays currently store three components.
  
  Members are constexpr so that everything is kept thread-local to the instance
  that created it; constexpr methods are implicitly inline.
  */
  public:
  // Simple constexpr accessors ===============================================
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
  static constexpr fmr::Local_int get_face_n ()
  noexcept{ return T::tris_n + T::quad_n;}
  static constexpr fmr::Local_int get_node_n () noexcept;
  //
  // Array constexpr pointers =================================================
  //
  // Natural element coordinates
  // Node coordinates use three components; supplied vertex coordinates do too.
  static constexpr bool get_elem_node_coor (F* node_coor,
    const F* vert_coor) noexcept {
    return calc_elem_node_coor(node_coor, vert_coor, 3);
  }
  static constexpr bool get_elem_node_coor (F* node_coor) noexcept {
    return calc_elem_node_coor(node_coor,
      vert_coor_ptr<T>(decltype(has_fixed_coor<T>(0)){}), coor_stride<T>(0));
  }
  //
  // Vertex arrays use the topology's stored coordinate stride.
  static constexpr const F* get_elem_vert_coor ()// vert major, dims minor
  noexcept { return vert_coor_ptr<T>(decltype(has_fixed_coor<T>(0)){});// transposed coor_vert
  }
  static constexpr const F* get_elem_coor_vert ()// dims major, vert minor
  noexcept { return coor_vert_ptr<T>(decltype(has_fixed_coor<T>(0)){});// (transposed vert_coor)
  }
  // Element constituent connectivity
  //NOTE get_vert_conn () is trivial = {0,1,...,get_vert_n()-1}
  //     get_node_conn () is trivial = {0,1,...,get_node_n()-1}
  //
  //NOTE the following are all only vertex connectivity (p==1) arrays.
  // Edge directions determine the order of higher-order edge nodes.
  // Solid face vertices wind counter-clockwise as viewed from outside.
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
  // Shape function and gradients =============================================
  static constexpr// zero if nothing (edge, face, volume) to integrate over
  fmr::Local_int get_poly_func_size (const fmr::Local_int intp_n=1u)
  noexcept { return (get_edge_n() + get_face_n() + get_vols_n() == 0) ? 0
    : intp_n * get_node_n ();
  }
  static constexpr
  fmr::Local_int get_poly_grad_size (
    const fmr::Local_int intp_n=1u, const fmr::Dim_int elem_d=T::elem_d)
  noexcept { return (get_edge_n() + get_face_n() + get_vols_n() == 0) ? 0
    : intp_n * get_node_n () * elem_d;
  }
  // P1-P3 Bars, Tris, Quad, Tets and Cube only. Points are packed as
  // [integration point][reference coordinate], with elem_d coordinates each.
  // Results are [point][node], [point][coordinate direction][node] for
  // get_poly_grad_node, and [point][node][coordinate direction] for
  // get_poly_node_grad. The dimension is the reference, not physical, dimension.
  // Buffers must not overlap and must hold intp_n*elem_d coordinates and
  // the number of results given by get_poly_func_size/get_poly_grad_size.
  // Returns false without writing when there are no results (including zero
  // points); otherwise node_n and elem_d must match this specialization and
  // invalid arguments throw std::invalid_argument.
  static constexpr
  bool get_poly_func_vals (F* poly_func, const F* intp,
    const fmr::Local_int node_n=get_node_n (),
    const fmr::Local_int intp_n=1u,
    const fmr::Dim_int   elem_d=T::elem_d);
  static constexpr
  bool get_poly_grad_node (F* poly_grad, const F* intp,
    const fmr::Local_int node_n=get_node_n (),
    const fmr::Local_int intp_n=1u,
    const fmr::Dim_int   elem_d=T::elem_d);
  static constexpr
  bool get_poly_node_grad (F* poly_grad, const F* intp,
    const fmr::Local_int node_n=get_node_n (),
    const fmr::Local_int intp_n=1u,
    const fmr::Dim_int   elem_d=T::elem_d);
  // Jacobians, inverses, and determinants ====================================
  // Jacobians are stored per integration point as row-major sims_d x elem_d.
  // jacs_dets_size reserves one additional scalar per point; for a rectangular
  // Jacobian this scalar is not an ordinary determinant. Inverted, or 
  // pseudo-inverted, it would normally be elem_d x sims_d.
  //
  // However, inverses are generally stored per integration point transposed as
  // sims_d x elem_d, and generally include an additional scalar per
  // integration point for the determinant of the non-inverted Jacobian.
  // Exceptions to the layout and extra scalar storage may be made in
  // low-dimensional cases for better performance.
  // This is why several variations are exposed here. The optimal storage layout
  // may depend on the specific element and problem dimensions and will be 
  // determined by automated performance self-testing of the variations.
  //
  static constexpr
  fmr::Local_int get_jacs_size (
    const fmr::Local_int intp_n=1u, const fmr::Dim_int sims_d=3u,
    const fmr::Dim_int elem_d=T::elem_d)
  noexcept {return intp_n * (sims_d*elem_d);}
  static constexpr
  fmr::Local_int get_jacs_dets_size (
    const fmr::Local_int intp_n=1, const fmr::Dim_int sims_d=3u,
    const fmr::Dim_int elem_d=T::elem_d)
  noexcept {return intp_n * (sims_d*elem_d + 1u);}
  //
  private:// ==================================================================
  template <class U> static
  auto has_fixed_coor (int) -> decltype((void) U::vert_coor, std::true_type{});
  template <class U> static std::false_type has_fixed_coor (...);
  template <class U> static constexpr
  const F* vert_coor_ptr (std::false_type) noexcept {
    return &U::template vert_coor<F>[0];
  }
  template <class U> static constexpr
  const F* vert_coor_ptr (std::true_type) noexcept {
    return &U::vert_coor[0];
  }
  template <class U> static constexpr
  const F* coor_vert_ptr (std::false_type) noexcept {
    return &U::template coor_vert<F>[0];
  }
  template <class U> static constexpr
  const F* coor_vert_ptr (std::true_type) noexcept {
    return &U::coor_vert[0];
  }
  template <class U> static constexpr
  auto coor_stride (int) noexcept -> decltype(U::sims_d) { return U::sims_d; }
  template <class U> static constexpr
  fmr::Dim_int coor_stride (...) noexcept { return 3; }
  static constexpr bool calc_elem_node_coor
    (F* node_coor, const F* vert_coor, fmr::Dim_int stride) noexcept;
  //
 };//end Elem
  namespace detail {
    enum class poly_output;
    template <class T, fmr::Dim_int P> struct poly_dispatch;
    template <class T, fmr::Dim_int P, typename F>
    constexpr bool eval_poly(F* out, const F* intp, fmr::Local_int node_n,
                            fmr::Local_int intp_n, fmr::Dim_int elem_d,
                            poly_output output);
  }// end detail namespace
  //
} } }//end femera::grid::elem:: namespace

#include "Elem.ipp"

//end FEMERA_HAS_GRID_ELEM_HPP
#endif
