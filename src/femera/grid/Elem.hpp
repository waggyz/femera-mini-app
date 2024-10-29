#ifndef FEMERA_HAS_GRID_ELEM_HPP
#define FEMERA_HAS_GRID_ELEM_HPP

//#include "../Grid.hpp"
#include "../../fmr/fmr.hpp"

namespace femera { namespace grid { namespace elem {

template <class T, typename fmr::Local_int P=1, typename fmr::Local_int D=3>
struct Elem {//TODO enable_if P>0 or for P in [1,2,3]
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
  constexpr fmr::Local_int get_elem_d () noexcept {return this_chld->elem_d;}
  constexpr fmr::Local_int get_vert_n () noexcept {return this_chld->vert_n;}
  constexpr fmr::Local_int get_vols_n () noexcept {return this_chld->vols_n;}
  constexpr fmr::Local_int get_edge_n () noexcept {return this_chld->edge_n;}
  constexpr fmr::Local_int get_tris_n () noexcept {return this_chld->tris_n;}
  constexpr fmr::Local_int get_quad_n () noexcept {return this_chld->quad_n;}
  //
  constexpr fmr::Geom_float get_edge_l () noexcept {return this_chld->edge_l;}
  constexpr fmr::Geom_float get_face_a () noexcept {return this_chld->face_a;}
  constexpr fmr::Geom_float get_elem_v () noexcept {return this_chld->elem_v;}
  //
  constexpr fmr::Local_int* get_spar_conn () noexcept {
    return this_chld->spar_conn;}
  constexpr fmr::Local_int* get_tris_conn () noexcept {
    return this_chld->tris_conn;}
  constexpr fmr::Local_int* get_quad_conn () noexcept {
    return this_chld->quad_conn;}
  constexpr fmr::Local_int* get_vert_conn () noexcept {
    return this_chld->vert_conn;}
  //
  constexpr fmr::Geom_float* get_vert_coor () noexcept {
    return this_chld->vert_coor;}
  constexpr fmr::Geom_float* get_coor_vert () noexcept {
    return this_chld->coor_vert;}
  //
  // Methods -----------------------------------------------------------------
  constexpr fmr::Local_int get_face_n () noexcept;
  constexpr fmr::Local_int get_node_n () noexcept;
  //
  constexpr fmr::Geom_float* get_node_coor () noexcept {
    return this_chld->vert_coor;// P=1, D=3
  }//FIXME Generate for P = 2,3, D=1,2.
  constexpr fmr::Geom_float* get_coor_node () noexcept {
    return this_chld->coor_vert;// P=1, D=3
  }//FIXME Generate for P = 2,3, D=1,2.
};

} } }//end femera::grid::elem:: namespace

#include "Elem.ipp"

//end FEMERA_HAS_GRID_ELEM_HPP
#endif
