#ifndef FEMERA_HAS_GRID_ELEM_TETS_HPP
#define FEMERA_HAS_GRID_ELEM_TETS_HPP

#include "Tris.hpp"
#include "Cube.hpp"

namespace femera { namespace grid { namespace elem {

struct Tets : public Elem<Tets> {//TODO Tet?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 3;
  static constexpr fmr::Local_int vert_n = 4;
  static constexpr fmr::Local_int edge_n = 6;
  static constexpr fmr::Local_int tris_n = 4;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 1;
  //
  static constexpr fmr::Geom_float edge_l  // total length of edges
    = 3.0 + 3.0*(std::sqrt(2.0));
  static constexpr fmr::Geom_float face_a // total surface area
    = 1.5 + std::sqrt(2.0) * std::sqrt(1.5);
  static constexpr fmr::Geom_float elem_v  // natural element volume
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
  };
  //const
  static constexpr
  fmr::Geom_float vert_coor [vert_n * sims_d ]// transposed coor_vert
    = {            //            3               //
                   // vertex    /|\              //
    0.0, 0.0, 0.0, // 0        / | \             //
    1.0, 0.0, 0.0, // 1       2--|--1            //
    0.0, 1.0, 0.0, // 2        \ | /     y z x   //
    0.0, 0.0, 1.0  // 3         \|/       \|/    //
  };               //            0         o     //
  static constexpr
  fmr::Geom_float coor_vert [sims_d * vert_n]// transposed vert_coor
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
  fmr::Local_int cube_conn_6 [4* 6]// 6-tet fill, tilable w/out rotation
    = {// These are all conformal and differ only by rotation.
    0,1,5,6,
    0,1,2,6,
    0,5,6,4,
    3,0,7,6,
    0,4,7,6,
    0,3,2,6
  };
  //fmr::Local_int tet5_cube_conn =[4* 6]//  5-tet fill
  fmr::Local_int cube_conn_5 [4* 5]// 5-tet fill, tilable with rotation
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
  static constexpr fmr::Phys_float a2 = 0.5854101966249685;
    // a2 = (5.0+3.0*std::sqrt(5.0))/20.0;
  static constexpr fmr::Phys_float b2 = 0.1381966011250105;
    // b2 = (5.0-std::sqrt(5.0))/20.0;
  static constexpr
  fmr::Phys_float intg_4_ptwt [4* intg_4_n] = {
    b2,b2,b2, 0.25/6.0,
    a2,b2,b2, 0.25/6.0,
    b2,a2,b2, 0.25/6.0,
    b2,b2,a2, 0.25/6.0
  };
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
  static constexpr
  fmr::Phys_float
    a0=0.0738349017262234    , a1=0.7784952948213300,
    b0=0.0937556561159491    , b1=0.4062443438840510,
    w0=0.0476331348432089/6.0, w1=0.1349112434378610/6.0;
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
  static constexpr fmr::Local_int intg_11_n = 11;//OLD B3
  //NOTE This converges tet20 meshes
  static constexpr fmr::Phys_float a3=0.3994035761667992;
  // a3 = (1.+std::sqrt(5./14.))/4.;
  static constexpr fmr::Phys_float b3=0.1005964238332008;
  // b3 = (1.-std::sqrt(5./14.))/4.;
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
  //------------------------- tet shape functions ----------------------------
  template <typename F> static inline// single or double
  void shap_func_4
  (F f[4], const F x[3]) {
    f[ 0] = 1.0 -x[0] -x[1] -x[2];
    f[ 1] = x[0];
    f[ 2] = x[1];
    f[ 3] = x[2];
    return;
  }
  template <typename F> static inline
  void shap_grad_4
  (F g[4u *3u], const F[]) {//  x[3] not used; //TODO Transpose g?
    g[ 0]=-1.0; g[ 1]=1.0; g[ 2]=0.0; g[ 3]=0.0;// dN/dx (natural coords)
    g[ 4]=-1.0; g[ 5]=0.0; g[ 6]=1.0; g[ 7]=0.0;// dN/dy
    g[ 8]=-1.0; g[ 9]=0.0; g[10]=0.0; g[11]=1.0;// dN/dz
  }
  template <typename F> static inline
  void shap_func_10
  (F f[10], const F x[3]) {
    const F L2=x[0], L3=x[1], L4=x[2];
    const F L1=1.0-L2-L3-L4;
    f[ 0] = 2.0*L1*L1 - L1;// Vertex nodes
    f[ 1] = 2.0*L2*L2 - L2;
    f[ 2] = 2.0*L3*L3 - L3;
    f[ 3] = 2.0*L4*L4 - L4;
    f[ 4] = 4.0*L1*L2;// Edge nodes
    f[ 5] = 4.0*L2*L3;
    f[ 6] = 4.0*L3*L1;
    f[ 7] = 4.0*L1*L4;
    f[ 8] = 4.0*L3*L4;
    f[ 9] = 4.0*L2*L4;
    return;
  }
  template <typename F> static inline
  void shap_grad_10 //TODO Return transpose?
  (F g[10u *3u], const F x[3]) {
    const F L2=x[0], L3=x[1], L4=x[2];
    const F L1=1.0-L2-L3-L4;
    // Term-by-term derivs
    const F L1r=-1.0, L2r=1.0, L3r=0.0, L4r=0.0;
    const F L1s=-1.0, L2s=0.0, L3s=1.0, L4s=0.0;
    const F L1t=-1.0, L2t=0.0, L3t=0.0, L4t=1.0;
    // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
    // r-derivs by product rule
    g[ 0] = 2.0*L1*L1r + 2.0*L1r*L1 - L1r;// Corner nodes
    g[ 1] = 2.0*L2*L2r + 2.0*L2r*L2 - L2r;
    g[ 2] = 2.0*L3*L3r + 2.0*L3r*L3 - L3r;
    g[ 3] = 2.0*L4*L4r + 2.0*L4r*L4 - L4r;
    g[ 4] = 4.0*L2*L1r + 4.0*L2r*L1;// Edge nodes
    g[ 5] = 4.0*L2*L3r + 4.0*L2r*L3;
    g[ 6] = 4.0*L3*L1r + 4.0*L3r*L1;
    g[ 7] = 4.0*L4*L1r + 4.0*L4r*L1;
    g[ 8] = 4.0*L3*L4r + 4.0*L3r*L4;
    g[ 9] = 4.0*L4*L2r + 4.0*L4r*L2;
    // s-derivs
    g[10] = 2.0*L1*L1s + 2.0*L1s*L1 - L1s;// Corner nodes
    g[11] = 2.0*L2*L2s + 2.0*L2s*L2 - L2s;
    g[12] = 2.0*L3*L3s + 2.0*L3s*L3 - L3s;
    g[13] = 2.0*L4*L4s + 2.0*L4s*L4 - L4s;
    g[14] = 4.0*L2*L1s + 4.0*L2s*L1;// Edge nodes
    g[15] = 4.0*L2*L3s + 4.0*L2s*L3;
    g[16] = 4.0*L3*L1s + 4.0*L3s*L1;
    g[17] = 4.0*L4*L1s + 4.0*L4s*L1;
    g[18] = 4.0*L3*L4s + 4.0*L3s*L4;
    g[19] = 4.0*L4*L2s + 4.0*L4s*L2;
    // t-derivs
    g[20] = 2.0*L1*L1t + 2.0*L1t*L1 - L1t;// Corner nodes
    g[21] = 2.0*L2*L2t + 2.0*L2t*L2 - L2t;
    g[22] = 2.0*L3*L3t + 2.0*L3t*L3 - L3t;
    g[23] = 2.0*L4*L4t + 2.0*L4t*L4 - L4t;
    g[24] = 4.0*L2*L1t + 4.0*L2t*L1;// Edge nodes
    g[25] = 4.0*L2*L3t + 4.0*L2t*L3;
    g[26] = 4.0*L3*L1t + 4.0*L3t*L1;
    g[27] = 4.0*L4*L1t + 4.0*L4t*L1;
    g[28] = 4.0*L3*L4t + 4.0*L3t*L4;
    g[29] = 4.0*L4*L2t + 4.0*L4t*L2;
    return;
  }
  template <typename F> static inline
  void shap_func_20 //TODO Return transpose?
  (F f[20], const F x[3]) {
    const F L2=x[0], L3=x[1], L4=x[2];
    const F L1=(1.0-L2-L3-L4);
    f[ 0]= 0.5* L1 *(3.0* L1 -1.)*(3.0* L1 -2.);// Vertex nodes;
    f[ 1]= 0.5* L2 *(3.0* L2 -1.)*(3.0* L2 -2.);
    f[ 2]= 0.5* L3 *(3.0* L3 -1.)*(3.0* L3 -2.);
    f[ 3]= 0.5* L4 *(3.0* L4 -1.)*(3.0* L4 -2.);
    // Edge nodes
    f[ 4]=4.5*( L1*L2 *(3.0*L1 -1.) );
    f[ 5]=4.5*( L1*L2 *(3.0*L2 -1.) );
    //
    f[ 6]=4.5*( L2*L3 *(3.0*L2 -1.) );
    f[ 7]=4.5*( L2*L3 *(3.0*L3 -1.) );
    //
    f[ 8]=4.5*( L1*L3 *(3.0*L3 -1.) );
    f[ 9]=4.5*( L1*L3 *(3.0*L1 -1.) );
    //
    f[10]=4.5*( L1*L4 *(3.0*L4 -1.) );
    f[11]=4.5*( L1*L4 *(3.0*L1 -1.) );
    //
    f[12]=4.5*( L3*L4 *(3.0*L4 -1.) );
    f[13]=4.5*( L3*L4 *(3.0*L3 -1.) );
    //
    f[14]=4.5*( L2*L4 *(3.0*L4 -1.) );
    f[15]=4.5*( L2*L4 *(3.0*L2 -1.) );
    // Face nodes 0,1,2, 0,1,3, 0,3,2, 1,2,3 
    f[16]=27.0*( L1*L2 *L3 );
    f[17]=27.0*( L1*L2 *L4 );
    f[18]=27.0*( L1*L4 *L3 );
    f[19]=27.0*( L2*L3 *L4 );
    return;
  }
  template <typename F> static inline
  void shap_grad_20 //TODO Return transpose?
  (F g[20u *3u], const F x[3]) {
    const F L2=x[0], L3=x[1], L4=x[2];
    const F L1=(1.0-L2-L3-L4);
    // Term-by-term derivs
    const F L1r=-1.0, L2r=1.0, L3r=0.0, L4r=0.0;
    const F L1s=-1.0, L2s=0.0, L3s=1.0, L4s=0.0;
    const F L1t=-1.0, L2t=0.0, L3t=0.0, L4t=1.0 ;
    // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
    g[ 0]= 0.5* L1r *(3.* L1 -1.)*(3.* L1 -2.)// corner nodes
         + 0.5* L1  *(3.* L1r   )*(3.* L1 -2.)
         + 0.5* L1  *(3.* L1 -1.)*(3.* L1r   );
    g[ 1]= 0.5* L2r *(3.* L2 -1.)*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2r   )*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2 -1.)*(3.* L2r   );
    g[ 2]= 0.5* L3r *(3.* L3 -1.)*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3r   )*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3 -1.)*(3.* L3r   );
    g[ 3]= 0.5* L4r *(3.* L4 -1.)*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4r   )*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4 -1.)*(3.* L4r   );
    // r-derivs, edge nodes
    g[ 4]=4.5*( L1r*L2 *(3.*L1 -1.) + L1 *L2r*(3.*L1 -1.) + L1 *L2 *(3.*L1r) );
    g[ 5]=4.5*( L1r*L2 *(3.*L2 -1.) + L1 *L2r*(3.*L2 -1.) + L1 *L2 *(3.*L2r) );
    //
    g[ 6]=4.5*( L2r*L3 *(3.*L2 -1.) + L2 *L3r*(3.*L2 -1.) + L2 *L3 *(3.*L2r) );
    g[ 7]=4.5*( L2r*L3 *(3.*L3 -1.) + L2 *L3r*(3.*L3 -1.) + L2 *L3 *(3.*L3r) );
    //
    g[ 8]=4.5*( L1r*L3 *(3.*L3 -1.) + L1 *L3r*(3.*L3 -1.) + L1 *L3 *(3.*L3r) );
    g[ 9]=4.5*( L1r*L3 *(3.*L1 -1.) + L1 *L3r*(3.*L1 -1.) + L1 *L3 *(3.*L1r) );
    //
    g[10]=4.5*( L1r*L4 *(3.*L4 -1.) + L1 *L4r*(3.*L4 -1.) + L1 *L4 *(3.*L4r) );
    g[11]=4.5*( L1r*L4 *(3.*L1 -1.) + L1 *L4r*(3.*L1 -1.) + L1 *L4 *(3.*L1r) );
    //
    g[12]=4.5*( L3r*L4 *(3.*L4 -1.) + L3 *L4r*(3.*L4 -1.) + L3 *L4 *(3.*L4r) );
    g[13]=4.5*( L3r*L4 *(3.*L3 -1.) + L3 *L4r*(3.*L3 -1.) + L3 *L4 *(3.*L3r) );
    //
    g[14]=4.5*( L2r*L4 *(3.*L4 -1.) + L2 *L4r*(3.*L4 -1.) + L2 *L4 *(3.*L4r) );
    g[15]=4.5*( L2r*L4 *(3.*L2 -1.) + L2 *L4r*(3.*L2 -1.) + L2 *L4 *(3.*L2r) );
    // face nodes 0,1,2, 0,1,3, 0,3,2, 1,2,3 
    g[16]=27.*( L1r*L2 *L3 + L1 *L2r*L3 + L1 *L2 *L3r);
    g[17]=27.*( L1r*L2 *L4 + L1 *L2r*L4 + L1 *L2 *L4r);
    g[18]=27.*( L1r*L4 *L3 + L1 *L4r*L3 + L1 *L4 *L3r);
    g[19]=27.*( L2r*L3 *L4 + L2 *L3r*L4 + L2 *L3 *L4r);
    // s-derivs
    g[20]= 0.5* L1s *(3.* L1 -1.)*(3.* L1 -2.)// corner nodes
         + 0.5* L1  *(3.* L1s   )*(3.* L1 -2.)
         + 0.5* L1  *(3.* L1 -1.)*(3.* L1s   );
    g[21]= 0.5* L2s *(3.* L2 -1.)*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2s   )*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2 -1.)*(3.* L2s   );
    g[22]= 0.5* L3s *(3.* L3 -1.)*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3s   )*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3 -1.)*(3.* L3s   );
    g[23]= 0.5* L4s *(3.* L4 -1.)*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4s   )*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4 -1.)*(3.* L4s   );
    // s-desivs, edge nodes
    g[24]=4.5*( L1s*L2 *(3.*L1 -1.) + L1 *L2s*(3.*L1 -1.) + L1 *L2 *(3.*L1s) );
    g[25]=4.5*( L1s*L2 *(3.*L2 -1.) + L1 *L2s*(3.*L2 -1.) + L1 *L2 *(3.*L2s) );
    //
    g[26]=4.5*( L2s*L3 *(3.*L2 -1.) + L2 *L3s*(3.*L2 -1.) + L2 *L3 *(3.*L2s) );
    g[27]=4.5*( L2s*L3 *(3.*L3 -1.) + L2 *L3s*(3.*L3 -1.) + L2 *L3 *(3.*L3s) );
    //
    g[28]=4.5*( L1s*L3 *(3.*L3 -1.) + L1 *L3s*(3.*L3 -1.) + L1 *L3 *(3.*L3s) );
    g[29]=4.5*( L1s*L3 *(3.*L1 -1.) + L1 *L3s*(3.*L1 -1.) + L1 *L3 *(3.*L1s) );
    //
    g[30]=4.5*( L1s*L4 *(3.*L4 -1.) + L1 *L4s*(3.*L4 -1.) + L1 *L4 *(3.*L4s) );
    g[31]=4.5*( L1s*L4 *(3.*L1 -1.) + L1 *L4s*(3.*L1 -1.) + L1 *L4 *(3.*L1s) );
    //
    g[32]=4.5*( L3s*L4 *(3.*L4 -1.) + L3 *L4s*(3.*L4 -1.) + L3 *L4 *(3.*L4s) );
    g[33]=4.5*( L3s*L4 *(3.*L3 -1.) + L3 *L4s*(3.*L3 -1.) + L3 *L4 *(3.*L3s) );
    //
    g[34]=4.5*( L2s*L4 *(3.*L4 -1.) + L2 *L4s*(3.*L4 -1.) + L2 *L4 *(3.*L4s) );
    g[35]=4.5*( L2s*L4 *(3.*L2 -1.) + L2 *L4s*(3.*L2 -1.) + L2 *L4 *(3.*L2s) );
    // face nodes 0,1,2, 0,1,3, 0,3,2, 1,2,3 
    g[36]=27.*( L1s*L2 *L3 + L1 *L2s*L3 + L1 *L2 *L3s);
    g[37]=27.*( L1s*L2 *L4 + L1 *L2s*L4 + L1 *L2 *L4s);
    g[38]=27.*( L1s*L4 *L3 + L1 *L4s*L3 + L1 *L4 *L3s);
    g[39]=27.*( L2s*L3 *L4 + L2 *L3s*L4 + L2 *L3 *L4s);
    // t-derivs
    g[40]= 0.5* L1t *(3.* L1 -1.)*(3.* L1 -2.)// corner nodes
         + 0.5* L1  *(3.* L1t   )*(3.* L1 -2.)
         + 0.5* L1  *(3.* L1 -1.)*(3.* L1t   );
    g[41]= 0.5* L2t *(3.* L2 -1.)*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2t   )*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2 -1.)*(3.* L2t   );
    g[42]= 0.5* L3t *(3.* L3 -1.)*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3t   )*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3 -1.)*(3.* L3t   );
    g[43]= 0.5* L4t *(3.* L4 -1.)*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4t   )*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4 -1.)*(3.* L4t   );
    // t-detivs, edge nodes
    g[44]=4.5*( L1t*L2 *(3.*L1 -1.) + L1 *L2t*(3.*L1 -1.) + L1 *L2 *(3.*L1t) );
    g[45]=4.5*( L1t*L2 *(3.*L2 -1.) + L1 *L2t*(3.*L2 -1.) + L1 *L2 *(3.*L2t) );
    //
    g[46]=4.5*( L2t*L3 *(3.*L2 -1.) + L2 *L3t*(3.*L2 -1.) + L2 *L3 *(3.*L2t) );
    g[47]=4.5*( L2t*L3 *(3.*L3 -1.) + L2 *L3t*(3.*L3 -1.) + L2 *L3 *(3.*L3t) );
    //
    g[48]=4.5*( L1t*L3 *(3.*L3 -1.) + L1 *L3t*(3.*L3 -1.) + L1 *L3 *(3.*L3t) );
    g[49]=4.5*( L1t*L3 *(3.*L1 -1.) + L1 *L3t*(3.*L1 -1.) + L1 *L3 *(3.*L1t) );
    //
    g[50]=4.5*( L1t*L4 *(3.*L4 -1.) + L1 *L4t*(3.*L4 -1.) + L1 *L4 *(3.*L4t) );
    g[51]=4.5*( L1t*L4 *(3.*L1 -1.) + L1 *L4t*(3.*L1 -1.) + L1 *L4 *(3.*L1t) );
    //
    g[52]=4.5*( L3t*L4 *(3.*L4 -1.) + L3 *L4t*(3.*L4 -1.) + L3 *L4 *(3.*L4t) );
    g[53]=4.5*( L3t*L4 *(3.*L3 -1.) + L3 *L4t*(3.*L3 -1.) + L3 *L4 *(3.*L3t) );
    //
    g[54]=4.5*( L2t*L4 *(3.*L4 -1.) + L2 *L4t*(3.*L4 -1.) + L2 *L4 *(3.*L4t) );
    g[55]=4.5*( L2t*L4 *(3.*L2 -1.) + L2 *L4t*(3.*L2 -1.) + L2 *L4 *(3.*L2t) );
    // face nodes 0,1,2, 0,1,3, 0,3,2, 1,2,3 
    g[56]=27.*( L1t*L2 *L3 + L1 *L2t*L3 + L1 *L2 *L3t);
    g[57]=27.*( L1t*L2 *L4 + L1 *L2t*L4 + L1 *L2 *L4t);
    g[58]=27.*( L1t*L4 *L3 + L1 *L4t*L3 + L1 *L4 *L3t);
    g[59]=27.*( L2t*L3 *L4 + L2 *L3t*L4 + L2 *L3 *L4t);
    return;
  }
//TODO singularity tetrahedron elements and integration rules
};

} } }//end femera::grid::elem:: namespace

#include "Tets.ipp"

//end FEMERA_HAS_GRID_ELEM_TETS_HPP
#endif
