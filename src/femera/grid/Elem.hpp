#ifndef FEMERA_HAS_GRID_ELEM_HPP
#define FEMERA_HAS_GRID_ELEM_HPP

//#include "../Grid.hpp"
#include "../../fmr/fmr.hpp"

#include <cmath>// std::sqrt () needed to calculate edge lengths

namespace femera { namespace grid { namespace elem {

// Forward-delare elements
class Tets; class Prmd; class Wdge; class Cube;
class Itri; class Iqud;
class Tris; class Quad;
class Spar;

template <class T, typename fmr::Local_int P=1, typename fmr::Local_int D=3,
  typename F=fmr::Geom_float>
struct Elem {//TODO enable_if P>0 or for P in [1,2,3]?
  /*
  Elements are defined in 3D by derived classes and reduced as needed to match
  the simulation spatial dimension template parameter (D).
  */
  T* this_chld = reinterpret_cast<T*>(this);
  // A static_cast only works for default template arguments.
  //
  // Simple accessors --------------------------------------------------------
  constexpr fmr::Local_int get_elem_p () noexcept {return P;}
  constexpr fmr::Local_int get_sims_d () noexcept {return D;}
  //
  constexpr fmr::Local_int get_elem_d () noexcept {return T::elem_d;}
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
  // Methods -----------------------------------------------------------------
  constexpr fmr::Local_int get_face_n () noexcept;
  constexpr fmr::Local_int get_node_n () noexcept;
  //
  //TODO Generate vert_conn, node_coor, coor_node for P=2,3, D=1,2.
  //TODO Consider enums for element type, order (# nodes), integration rule.
  template <fmr::Local_int tP=P,
    typename std::enable_if <(tP == 1)>::value>// P=1, D does not matter
  constexpr const fmr::Local_int* get_node_conn () noexcept {
   return T::vert_conn;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <(tP == 1)>::value,
    typename std::enable_if <(tD == 3)>::value>// P=1, D=3
  constexpr const F* get_node_coor () noexcept {
   return T::vert_coor;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <(tP == 1)>::value,
    typename std::enable_if <(tD == 3)>::value>// P=1, D=3
  constexpr const F* get_coor_node () noexcept {
    return T::coor_vert;
  }
  template <fmr::Local_int tP=P, fmr::Local_int tD=D,
    typename std::enable_if <(tP == 2)>::value,
    typename std::enable_if <(tD == 3)>::value>// P=2, D=3
  constexpr const F* get_coor_node () noexcept {
    return T::coor_node_2;
  }
  static inline
  fmr::Local_int jacd_size (const fmr::Local_int intp_n=1) noexcept;
#if 0
  template <typename = typename std::enable_if
    <std::is_same< F, double >::value >::type>
  static inline
  F* jacd
    (F* jacd, const fmr::math::Intg_rule) noexcept;
#endif
};

//TODO These are used for testing. Should they live somewhere else?
std::string tri2_norm_str
 (const fmr::Geom_float* p1,
  const fmr::Geom_float* p2,
  const fmr::Geom_float* p3);
std::string tri3_norm_str
 (const fmr::Geom_float* p1,
  const fmr::Geom_float* p2,
  const fmr::Geom_float* p3);

} } }//end femera::grid::elem:: namespace

#include "Elem.ipp"

//end FEMERA_HAS_GRID_ELEM_HPP
#endif
