#ifndef FEMERA_HAS_GRID_ELEM_TETS_HPP
#define FEMERA_HAS_GRID_ELEM_TETS_HPP

#include "Tris.hpp"
#include "Cube.hpp"

#include <cmath> // std::sqrt()

namespace femera { namespace grid { namespace elem {

struct Tets : public Elem<Tets> {//TODO Tet?
  public://TODO make protected or private with friend class Elem?
  static constexpr fmr::Local_int sims_d = 3;//TODO Remove from here?
  static constexpr fmr::Local_int elem_d = 3;
  static constexpr fmr::Local_int vert_n = 4;
  static constexpr fmr::Local_int edge_n = 6;
  static constexpr fmr::Local_int tris_n = 4;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 1;
  //
  static constexpr fmr::Geom_float edge_l // total length of edges
    = 3.0 + 3.0 * std::sqrt (2.0);
  static constexpr fmr::Geom_float face_a // total surface area
    = 1.5 + std::sqrt (2.0) * std::sqrt (1.5);
  static constexpr fmr::Geom_float elem_v // natural element volume
    = 1.0 / 6.0;
  //
  //NOTE Gmsh element conventions
  static constexpr
  fmr::Local_int vert_conn [vert_n] = {0,1,2,3};
  static constexpr
  fmr::Local_int tris_conn [Tris::vert_n * tris_n]
    = { //NOTE the normals point inward. This might be wrong.
    0,1,2, 0,3,1, 0,2,3, 1,3,2
  };
  static constexpr
  fmr::Local_int* quad_conn = nullptr;
  static constexpr
  fmr::Local_int spar_conn [Spar::vert_n * edge_n]
    = {
    0,1, 1,2, 2,0, 0,3, 2,3, 1,3
//  0,1, 1,2, 2,0, 0,3, 1,3, 2,3//TODO switch in the shape functions?
  };
  static constexpr
  double vert_coor [vert_n * sims_d ]// transposed coor_vert
    = {            //            3               //
                   // vertex    /|\              //
    0.0, 0.0, 0.0, // 0        / | \             //
    1.0, 0.0, 0.0, // 1       2--|--1            //
    0.0, 1.0, 0.0, // 2        \ | /     y z x   //
    0.0, 0.0, 1.0  // 3         \|/       \|/    //
  };               //            0         o     //
  static constexpr
  double coor_vert [sims_d * vert_n]// transposed vert_coor
    = {
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
  };
  static constexpr
  double tet10_coor [sims_d * (vert_n + edge_n) ]// transposed coor_node_2
    = {
    0.0, 0.0, 0.0,// vertes nodes
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    //
    0.5, 0.0, 0.0,// bottom triangle edge nodes
    0.5, 0.5, 0.0,
    0.0, 0.5, 0.0,
    //
    0.0, 0.0, 0.5,// apex edge nodes
    0.0, 0.5, 0.5,//TOD switch these last two?
    0.5, 0.0, 0.5
  };
  static constexpr
  double coor_tet10 [sims_d * (vert_n + edge_n)]// transposed node_coor_2
    = {
    0.0, 1.0, 0.0, 0.0,   0.5, 0.5, 0.0,   0.0, 0.0, 0.5,
    0.0, 0.0, 1.0, 0.0,   0.0, 0.5, 0.5,   0.0, 0.5, 0.0,
    0.0, 0.0, 0.0, 1.0,   0.0, 0.0, 0.0,   0.5, 0.5, 0.5
  };//0,  1,   2,   3,     4,   5,   6,   7,   8,   9 TODO switch columns 8 & 9?
  static constexpr
  float vert_coor_f [vert_n * sims_d ]// transposed coor_vert
    = {
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0 
  };
  static constexpr
  float coor_vert_f [sims_d * vert_n]// transposed vert_coor
    = {
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
  };
#if 0
  fmr::Geom_float cube_coor [3* 8]// same as cube elem below
    = {
                    // vertex   7---------6                //
     0.0, 0.0, 0.0, // 0       /|        /|                //
     1.0, 0.0, 0.0, // 1      / |       / |                //
     1.0, 1.0, 0.0, // 2     /  |      /  |                //
     0.0, 1.0, 0.0, // 3    4---------5   |                //
                    //      |   |     |   |                //
     0.0, 0.0, 1.0, // 4    |   3-----|---2                //
     1.0, 0.0, 1.0, // 5    |  /      |  /      z  y       //
     1.0, 1.0, 1.0, // 6    | /       | /       | /        //
     0.0, 1.0, 1.0  // 7    |/        |/        |/         //
                    //      0---------1         o--x       //
  };
#endif
  static constexpr
  fmr::Local_int cube_conn_6tet [4* 6]// 6-tet fill, tilable w/out rotation
    = {// These are all conformal and differ only by rotation.
    0,1,5,6,
    0,1,2,6,
    0,5,6,4,
    3,0,7,6,
    0,4,7,6,
    0,3,2,6
  };
  //fmr::Local_int tet5_cube_conn =[4* 6]//  5-tet fill
  static constexpr
  fmr::Local_int cube_conn_5tet [4* 5]// 5-tet fill, tilable with rotation
    = {
    1,3,4,6,// This tet is twice the volume of the rest.
    0,1,3,4,// Identical to the natural tet (identity Jacobian).
    2,3,1,6,
    5,6,1,4,
    7,6,4,3
  };
  //TODO tets patch test mesh
  //--------------------------- tet integration ------------------------------
  // Tetrahedral integration points and weights
  // for linear-shaped tetrahedra (edge nodes are interpolated),
  // the Jacobian is constant and independent of the int pt locations.
  // So, only ONE 3x3+1 (Jacobian+det) is needed for each element
  // regardless of tet element order.
  // The volume of a natural tet is 1/6,
  // and multiplied into the integration rules here.
  //
  static constexpr fmr::Local_int intg_1_n = 1;// Preferred P1
  static constexpr
  fmr::Phys_float intg_1_ptwt [4* intg_1_n] = {
    0.25,0.25,0.25, 1.0/6.0
  };
  static constexpr fmr::Local_int intg_4_n = 4;// Preferred P2
  // a2 = (5.0+3.0*std::sqrt(5.0))/20.0 = 0.5854101966249685;
  // b2 = (5.0-    std::sqrt(5.0))/20.0 = 0.1381966011250105;
#define a2 (5.0+3.0*std::sqrt(5.0))/20.0
#define b2 (5.0-std::sqrt(5.0))/20.0
  static constexpr
  fmr::Phys_float intg_4_ptwt [4* intg_4_n] = {
    b2,b2,b2, 0.25/6.0,
    a2,b2,b2, 0.25/6.0,
    b2,a2,b2, 0.25/6.0,
    b2,b2,a2, 0.25/6.0
  };
#undef a2
#undef b2
  static constexpr fmr::Local_int intg_5_n = 5;// Alternate P2
  //NOTE tet20s don't converge with 5-point rule
  //NOTE Triple-checked these 5-point rule values
  static constexpr
  fmr::Phys_float intg_5_ptwt [4* intg_5_n] = {
    0.25   , 0.25   , 0.25   ,-4.0/ 30.0,
    0.5    , 1.0/6.0, 1.0/6.0, 9.0/120.0,
    1.0/6.0, 0.5    , 1.0/6.0, 9.0/120.0,
    1.0/6.0, 1.0/6.0, 0.5    , 9.0/120.0,
    1.0/6.0, 1.0/6.0, 1.0/6.0, 9.0/120.0
  };
  static constexpr fmr::Local_int intg_10_n = 10;//Preferred B3
  // From Lee Shunn, Frank Ham, Symmetric quadrature rules for tetrahedra
  // based on a cubic close-packed lattice arrangement, 2012
#define a0 0.0738349017262234
#define a1 0.7784952948213300
#define b0 0.0937556561159491
#define b1 0.4062443438840510
#define w0 0.0476331348432089/6.0
#define w1 0.1349112434378610/6.0
  static constexpr
  fmr::Phys_float intg_10_ptwt [4* intg_10_n] = {
    a0,a0,a0, w0,
    a1,a0,a0, w0,
    a0,a1,a0, w0,
    a0,a0,a1, w0,
    b1,b0,b0, w1,
    b0,b1,b0, w1,
    b0,b0,b1, w1,
    b0,b1,b1, w1,
    b1,b0,b1, w1,
    b1,b1,b0, w1
  };
#undef a0
#undef a1
#undef b0
#undef b1
#undef w0
#undef w1
  static constexpr fmr::Local_int intg_11_n = 11;//OLD B3
  // This converges tet20 meshes
  // a3 = (1.0+std::sqrt(5.0/14.0))/4.0 = 0.3994035761667992
  // b3 = (1.0-std::sqrt(5.0/14.0))/4.0 = 0.1005964238332008
#define a3 (1.0+std::sqrt(5.0/14.0))/4.0
#define b3 (1.0-std::sqrt(5.0/14.0))/4.0
  static constexpr
  fmr::Phys_float intg_11_ptwt [4* intg_11_n] = {
     0.25    , 0.25    , 0.25    , -74.0/ 5625.0,
     1.0/14.0, 1.0/14.0, 1.0/14.0, 343.0/45000.0,
    11.0/14.0, 1.0/14.0, 1.0/14.0, 343.0/45000.0,
     1.0/14.0,11.0/14.0, 1.0/14.0, 343.0/45000.0,
     1.0/14.0, 1.0/14.0,11.0/14.0, 343.0/45000.0,
    b3,a3,a3, 56.0/2250.0,
    a3,b3,a3, 56.0/2250.0,
    a3,a3,b3, 56.0/2250.0,
    a3,b3,b3, 56.0/2250.0,
    b3,a3,b3, 56.0/2250.0,
    b3,b3,a3, 56.0/2250.0
  };
#undef a3
#undef b3
  //------------------------- tet shape functions ----------------------------
  //TODO Return shape function/gradient transposed?
  template <typename F> static inline// single or double
  F* shap_func_4 (F f[4u], const F x[3u]);
  template <typename F> static inline
  F* shap_grad_4 (F g[4u *3u], const F[]);// x[3u] not used; result is constant
  template <typename F> static inline
  F* shap_func_10 (F f[10u], const F x[3u]);
  template <typename F> static inline
  F* shap_grad_10 (F g[10u *3u], const F x[3u]);
  template <typename F> static inline
  F* shap_func_20 (F f[20u], const F x[3u]);
  template <typename F> static inline
  F* shap_grad_20 (F g[20u *3u], const F x[3u]) ;
  //TODO singularity tetrahedron element shape functions and integration rules
};

} } }//end femera::grid::elem:: namespace

#include "Tets.ipp"

//end FEMERA_HAS_GRID_ELEM_TETS_HPP
#endif
