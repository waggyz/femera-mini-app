#ifndef FEMERA_HAS_GRID_ELEM_HPP
#define FEMERA_HAS_GRID_ELEM_HPP

//#include "../Grid.hpp"
#include "../../fmr/fmr.hpp"
//#include "elem/Tets.hpp"
//#include "elem/Prmd.hpp"
//#include "elem/Wdge.hpp"
//#include "elem/Itri.hpp"
//#include "elem/Iqud.hpp"

#include <cmath>// std::sqrt () needed to calculate edge lengths

#if 1
namespace fmr { namespace grid { namespace elem { //TODO move to src/fmr/

struct Elem_args {
public:
  fmr::Local_int node_n;
  fmr::Local_int intp_n;
  fmr::Local_int sims_d;
};

template <class T, Elem_args const &A, typename F=fmr::Geom_float>
class Elem_test {
  private:
  static constexpr fmr::Local_int node_n = A.node_n;
  static constexpr fmr::Local_int intp_n = A.intp_n;
  static constexpr fmr::Local_int sims_d = A.sims_d;
  public:
  template <typename fmr::Local_int D=sims_d>
  constexpr typename std::enable_if <D == 3, fmr::Local_int>::type
  test_vert_n () noexcept {
    return T::vert_n;
  }
//    typename std::enable_if <tP == 1>::value,

};
} } }//end fmr::grid::elem:: namespace
#endif


// Supported element examples
//static constexpr Elem_args tet10_args   = {10, 4, 3};
//static constexpr Elem_args tri6_2d_args = { 6, 3, 2};
//static constexpr Elem_args tri6_3d_args = { 6, 3, 3};

namespace femera { namespace grid { namespace elem {

// Forward-delare element shapes
class Tets; class Prmd; class Wdge; class Cube;
class Itri; class Iqud;
class Tris; class Quad;
class Spar;

template <class T, typename fmr::Local_int P=1, typename fmr::Local_int D=3,
  typename F=fmr::Geom_float>
class Elem {//TODO enable_if P>0 or for P in [1,2,3]?
  //FIXME higher-order elements are identified by node_n <N>, not elem_p <P>.
  //TODO one more Elem template parameter for integration rule <R>?
  //TODO Consider enums for element type, order (# nodes), integration rule.
  //TODO Change to template <class T, Elem_args A, typename F>, as above?
  //     But, does this allow enable_if on the struct content? - YES
  /*
  Elements are defined in 3D by derived classes and reduced as needed to match
  the simulation spatial dimension template parameter (D).
  
  Members are constexpr so that everything is kept thread-local to the instance
  that created it.
  */
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
  template <typename tF=F,
    typename std::enable_if <std::is_same< tF, double >::value >::type>
  constexpr const F* get_vert_coor () noexcept {
    return T::vert_coor;
  }
  template <typename tF=F,
    typename std::enable_if <std::is_same< tF, double >::value >::type>
  constexpr const F* get_coor_vert () noexcept {
    return T::coor_vert;
  }
  template <typename tF=F,
    typename std::enable_if <std::is_same< tF, float >::value >::type>
  constexpr const F* get_vert_coor () noexcept {
    return T::vert_coor_f;
  }
  template <typename tF=F,
    typename std::enable_if <std::is_same< tF, float >::value >::type>
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
  //TODO Generate node_coor, coor_node for P=2,3; D=1,2.
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
