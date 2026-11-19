#ifndef FEMERA_HAS_GRID_ELEM_IPIN_HPP
#define FEMERA_HAS_GRID_ELEM_IPIN_HPP

#include "../Elem.hpp"

namespace femera { namespace grid { namespace elem {

struct Ipin : public Elem<Ipin> {
  template <class T, fmr::Dim_int P, typename F>
  friend class Elem;
private:
  static constexpr fmr::math::Poly poly_type = fmr::math::Poly::None;
  //
  static constexpr fmr::Dim_int   elem_d = 1;
  static constexpr fmr::Local_int vert_n = 2;
  static constexpr fmr::Local_int edge_n = 0;
  static constexpr fmr::Local_int tris_n = 0, tri3_face_node_n = 0;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 0;
  //
  static constexpr fmr::Geom_float edge_l = 0.0;
  static constexpr fmr::Geom_float face_a = 0.0;
  static constexpr fmr::Geom_float elem_v = 0.0;
  //
  static constexpr fmr::Local_int* bars_vert_conn = nullptr;
  static constexpr fmr::Local_int* tris_vert_conn = nullptr;
  static constexpr fmr::Local_int* quad_vert_conn = nullptr;

  template <typename F> static constexpr
  F vert_coor [vert_n * 3u] = {// vert major, dims minor (transposed coor_vert)
    F(0.0), F(0.0), F(0.0),
    F(0.0), F(0.0), F(0.0) 
  };
  template <typename F> static constexpr
  F coor_vert [vert_n * 3u] = {// dims major, vert minor (transposed vert_coor)
    F(0.0), F(0.0),
    F(0.0), F(0.0),
    F(0.0), F(0.0)
  };
};
/* Notionally, shape function and gradient matrices do make sense for a
   1D pin element. The matrix below represents stiffness contributions from a
   simple linear force-displacement relationship along each axis for the
   two nodes of the P1 Ipin element in 3D space, and readilly reduces to
   2D and 1D spaces.

        f1x f2x f1y f2y f1z f2z
        =
         kx -kx  0   0   0   0     u1x
        -kx  kx  0   0   0   0     u2x
         0   0   ky -ky  0   0     u1y
         0   0  -ky  ky  0   0     u2y
         0   0   0   0   kz -kz    u1z
         0   0   0   0  -kz  kz    u2z

   Isotropic response: kx = ky = kz

   An Ipin element could be placed along structural edges to model axial
   stiffness contributions. The formulation above is fine for P1 edges.
   What are the implications for higher-order Ipin elements?

   Will 2D (Ibar) and 3D (Itri and Iqud) elements follow a similar pattern for
   traction-seperation relationships, e.g., cohesive elements? Do the
   higher-dimensional element formulations reduce naturally to this Ipin
   formulation?

 */

} } }//end femera::grid::elem:: namespace

#include "Ipin.ipp"

//end FEMERA_HAS_GRID_ELEM_IPIN_HPP
#endif
