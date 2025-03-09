#ifndef FEMERA_HAS_GRID_ELEM_HPP
#define FEMERA_HAS_GRID_ELEM_HPP

#include "../../fmr/fmr.hpp"

#if 1
// Elem_test *****************************************************************
namespace fmr { namespace grid { namespace elem { //TODO move to src/fmr/

struct Elem_args {
public:
  fmr::Local_int node_n;// N
  fmr::Local_int intp_n;// G
  fmr::Local_int sims_d;// D
};
template <class E, Elem_args const &A, typename F=fmr::Geom_float>
class Elem_test {
private:
  static constexpr fmr::Local_int node_n = A.node_n;
  static constexpr fmr::Local_int intp_n = A.intp_n;
  static constexpr fmr::Local_int sims_d = A.sims_d;
public:
  template <typename fmr::Local_int D=sims_d>
  constexpr typename std::enable_if <D==3, fmr::Local_int>::type
  test_vert_n () noexcept {return E::vert_n;}
};

} } }//end fmr::grid::elem:: namespace
// ***************************************************************************
#endif

namespace femera { namespace grid { namespace elem {

// Forward-delare element shapes
class Spar;
class Line;//TODO 1D interface element embedded in 2D or 3D space
class Itri; class Iqud;// 2D interface elements embedded in 3D space
class Tris; class Quad;
class Tets; class Prmd; class Wdge; class Cube;
class User;//TODO user element

template <class T, typename fmr::Local_int P=1, typename fmr::Local_int D=3,
  typename F=fmr::Geom_float>
class Elem {//TODO enable_if P>0 or for P in [1,2,3]?
  /*
  Elements are defined in 3D by derived classes and reduced as needed to match
  the simulation spatial dimension template parameter (D).
  
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
  private:
  T* this_chld = reinterpret_cast<T*>(this);
  // A static_cast only works for default template arguments.
  //
  public:
  // Simple accessors --------------------------------------------------------
  // constexpr functions are implicitly inline
  constexpr fmr::Local_int get_elem_p () noexcept {return P;}
  constexpr fmr::Local_int get_sims_d () noexcept {return D;}
  constexpr fmr::Local_int get_elem_d () noexcept {
    static_assert( T::elem_d <= D, //NOTE only checked if this function used.
      "Element dimension cannot exceed simulation spatial dimension.");
    return T::elem_d;}
  constexpr fmr::Local_int get_vert_n () noexcept {return T::vert_n;}
  constexpr fmr::Local_int get_vols_n () noexcept {return T::vols_n;}
  constexpr fmr::Local_int get_edge_n () noexcept {return T::edge_n;}
  constexpr fmr::Local_int get_tris_n () noexcept {return T::tris_n;}
  constexpr fmr::Local_int get_quad_n () noexcept {return T::quad_n;}
  //
  constexpr F get_edge_l () noexcept {return F(T::edge_l);}
  constexpr F get_face_a () noexcept {return F(T::face_a);}
  constexpr F get_elem_v () noexcept {return F(T::elem_v);}
  //
  constexpr const fmr::Local_int* get_spar_conn () noexcept {
    return T::spar_conn;}
  constexpr const fmr::Local_int* get_tris_conn () noexcept {
    return T::tris_conn;}
  constexpr const fmr::Local_int* get_quad_conn () noexcept {
    return T::quad_conn;}
  constexpr const fmr::Local_int* get_vert_conn () noexcept {
    return T::vert_conn;}
  //
  // Natural element vertex coordinates --------------------------------------
  template <typename Ft=F,
    typename std::enable_if <std::is_same<Ft, double>::value>::type* = nullptr>
  constexpr const F* get_vert_coor () noexcept {
    return T::vert_coor;
  }
  template <typename Ft=F,
    typename std::enable_if <std::is_same<Ft, double>::value>::type* = nullptr>
  constexpr const F* get_coor_vert () noexcept {
    return T::coor_vert;
  }
  template <typename Ft=F,
    typename std::enable_if <std::is_same<Ft, float>::value>::type* = nullptr>
  constexpr const F* get_vert_coor () noexcept {
    return T::vert_coor_f;
  }
  template <typename Ft=F,
    typename std::enable_if <std::is_same< Ft,float>::value>::type* = nullptr>
  constexpr const F* get_coor_vert () noexcept {
    return T::coor_vert_f;
  }
  // Non-trivial member functions --------------------------------------------
  constexpr
  fmr::Local_int get_face_n () noexcept;
  constexpr
  fmr::Local_int get_node_n () noexcept;
  constexpr
  fmr::Local_int jacd_size (const fmr::Local_int intp_n=1) noexcept;
  constexpr
  fmr::Local_int shpg_size (const fmr::Local_int intp_n=1) noexcept;
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
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 1>::value,
    typename std::enable_if <tD == 3>::value>// P=1, D=3
  constexpr const F* get_node_coor () noexcept {
   return T::vert_coor;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 1>::value,
    typename std::enable_if <tD == 3>::value>// P=1, D=3
  constexpr const F* get_coor_node () noexcept {
    return T::coor_vert;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 2>::value,
    typename std::enable_if <tD == 3>::value>// P=2, D=3
  constexpr const F* get_node_coor () noexcept {
   return T::tet10_coor;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 2>::value,
    typename std::enable_if <tD == 3>::value>// P=2, D=3
  constexpr const F* get_coor_node () noexcept {
    return  T::coor_tet10;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 3>::value,
    typename std::enable_if <tD == 3>::value>// P=3, D=3
  constexpr const F* get_node_coor () noexcept {
   return T::tet20_coor;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <tP == 3>::value,
    typename std::enable_if <tD == 3>::value>// P=3, D=3
  constexpr const F* get_coor_node () noexcept {
    return  T::coor_tet20;
  }
  // Integration points and weights (default) --------------------------------
  //
  // Shape functions and gradients -------------------------------------------
  //
};

//TODO These are used for testing. Should they live somewhere else?
std::string tri2_norm_str // in 3D
 (const fmr::Geom_float* pt1,
  const fmr::Geom_float* pt2,
  const fmr::Geom_float* pt3);
std::string tri3_norm_str // in 3D
 (const fmr::Geom_float* pt1,
  const fmr::Geom_float* pt2,
  const fmr::Geom_float* pt3);

} } }//end femera::grid::elem:: namespace

#include "Elem.ipp"

//end FEMERA_HAS_GRID_ELEM_HPP
#endif
