#ifndef FEMERA_HAS_GRID_ELEM_TETS_IPP
#define FEMERA_HAS_GRID_ELEM_TETS_IPP

namespace femera { namespace grid { namespace elem {
/*
C++11 requires redundant declaration/definition for constexpr arrays to 
reference them later.
*/
constexpr fmr::Local_int Tets::vols_n;// optional for basic types

constexpr fmr::Local_int Tets::tris_conn [];
//constexpr fmr::Local_int Tets::quad_conn [];
constexpr fmr::Local_int Tets::spar_conn [];
constexpr fmr::Geom_float Tets::vert_coor [];
constexpr fmr::Geom_float Tets::coor_vert [];
constexpr fmr::Geom_float Tets::coor_tet10 [];
constexpr fmr::Geom_float Tets::tet10_coor [];
constexpr fmr::Geom_float Tets::coor_tet20 [];
constexpr fmr::Geom_float Tets::tet20_coor [];

constexpr fmr::Phys_float Tets::intg_1_ptwt [];
constexpr fmr::Phys_float Tets::intg_4_ptwt [];
constexpr fmr::Phys_float Tets::intg_5_ptwt [];
constexpr fmr::Phys_float Tets::intg_10_ptwt [];
constexpr fmr::Phys_float Tets::intg_11_ptwt [];

// Tets::shap_func_X and Tets::shap_grad_N evaluate the shape functions or
// gradients at 3D natural coordinate point x.

template <typename F> inline// single or double precision
F* Tets::shap_func_4
(F f[4u], const F x[3u]) {
  f[ 0] = 1.0-x[0]-x[1]-x[2];
  f[ 1] = x[0];
  f[ 2] = x[1];
  f[ 3] = x[2];
  return f;
}
template <typename F> inline
F* Tets::shap_grad_4
(F g[4u *3u], const F[]) {//  x[3u] not used (constant gradient)
  g[ 0]=-1.0; g[ 1]=1.0; g[ 2]=0.0; g[ 3]=0.0;// dN/dx (natural coords)
  g[ 4]=-1.0; g[ 5]=0.0; g[ 6]=1.0; g[ 7]=0.0;// dN/dy
  g[ 8]=-1.0; g[ 9]=0.0; g[10]=0.0; g[11]=1.0;// dN/dz
  return g;
}
template <typename F> inline
F* Tets::shap_func_10
(F f[10u], const F x[3u]) {
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
  return f;
}
template <typename F> inline
F* Tets::shap_grad_10
(F g[10u *3u], const F x[3u]) {
  const F L2=x[0], L3=x[1], L4=x[2];
  const F L1=1.0-L2-L3-L4;
  // Term-by-term derivs
  const F L1r=-1.0, L2r=1.0, L3r=0.0, L4r=0.0;
  const F L1s=-1.0, L2s=0.0, L3s=1.0, L4s=0.0;
  const F L1t=-1.0, L2t=0.0, L3t=0.0, L4t=1.0;
  // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
  // r-derivs by product rule
  g[ 0] = 2.0*L1*L1r + 2.0*L1r*L1 - L1r;// Vertex nodes
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
  g[10] = 2.0*L1*L1s + 2.0*L1s*L1 - L1s;// Vertex nodes
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
  g[20] = 2.0*L1*L1t + 2.0*L1t*L1 - L1t;// Vertex nodes
  g[21] = 2.0*L2*L2t + 2.0*L2t*L2 - L2t;
  g[22] = 2.0*L3*L3t + 2.0*L3t*L3 - L3t;
  g[23] = 2.0*L4*L4t + 2.0*L4t*L4 - L4t;
  g[24] = 4.0*L2*L1t + 4.0*L2t*L1;// Edge nodes
  g[25] = 4.0*L2*L3t + 4.0*L2t*L3;
  g[26] = 4.0*L3*L1t + 4.0*L3t*L1;
  g[27] = 4.0*L4*L1t + 4.0*L4t*L1;
  g[28] = 4.0*L3*L4t + 4.0*L3t*L4;
  g[29] = 4.0*L4*L2t + 4.0*L4t*L2;
  return g;
}
template <typename F> inline
F* Tets::shap_func_20
(F f[20u], const F x[3u]) {
  const F L2=x[0], L3=x[1], L4=x[2];
  const F L1=(1.0-L2-L3-L4);
  f[ 0]= 0.5* L1 *(3.0* L1 -1.0)*(3.0* L1 -2.0);// Vertex nodes;
  f[ 1]= 0.5* L2 *(3.0* L2 -1.0)*(3.0* L2 -2.0);
  f[ 2]= 0.5* L3 *(3.0* L3 -1.0)*(3.0* L3 -2.0);
  f[ 3]= 0.5* L4 *(3.0* L4 -1.0)*(3.0* L4 -2.0);
  // Edge nodes
  f[ 4]=4.5*( L1*L2 *(3.0*L1 -1.0) );
  f[ 5]=4.5*( L1*L2 *(3.0*L2 -1.0) );
  //
  f[ 6]=4.5*( L2*L3 *(3.0*L2 -1.0) );
  f[ 7]=4.5*( L2*L3 *(3.0*L3 -1.0) );
  //
  f[ 8]=4.5*( L1*L3 *(3.0*L3 -1.0) );
  f[ 9]=4.5*( L1*L3 *(3.0*L1 -1.0) );
  //
  f[10]=4.5*( L1*L4 *(3.0*L4 -1.0) );
  f[11]=4.5*( L1*L4 *(3.0*L1 -1.0) );
  //
  f[12]=4.5*( L3*L4 *(3.0*L4 -1.0) );
  f[13]=4.5*( L3*L4 *(3.0*L3 -1.0) );
  //
  f[14]=4.5*( L2*L4 *(3.0*L4 -1.0) );
  f[15]=4.5*( L2*L4 *(3.0*L2 -1.0) );
  // Face nodes 0,1,2, 0,1,3, 0,3,2, 1,2,3 
  f[16]=27.0*( L1*L2 *L3 );
  f[17]=27.0*( L1*L2 *L4 );
  f[18]=27.0*( L1*L4 *L3 );
  f[19]=27.0*( L2*L3 *L4 );
  return f;
}
template <typename F> inline
F* Tets::shap_grad_20
(F g[20u *3u], const F x[3u]) {
  const F L2=x[0], L3=x[1], L4=x[2];
  const F L1=(1.0-L2-L3-L4);
  // Term-by-term derivs
  const F L1r=-1.0, L2r=1.0, L3r=0.0, L4r=0.0;
  const F L1s=-1.0, L2s=0.0, L3s=1.0, L4s=0.0;
  const F L1t=-1.0, L2t=0.0, L3t=0.0, L4t=1.0 ;
  // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
  g[ 0]= 0.5* L1r *(3.* L1 -1.)*(3.* L1 -2.)// Vertex nodes
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
  g[20]= 0.5* L1s *(3.* L1 -1.)*(3.* L1 -2.)// Vertex nodes
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
  g[40]= 0.5* L1t *(3.* L1 -1.)*(3.* L1 -2.)// Vertex nodes
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
  // t-derivs, edge nodes
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
  return g;
}

} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_TETS_IPP
#endif
