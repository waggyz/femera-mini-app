#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>

#define _MM256_TRANSPOSE3_PD(row0, row1, row2) {          \
  __m256d tmp0 = _mm256_unpacklo_pd(row0, row1);        \
  __m256d tmp1 = _mm256_unpackhi_pd(row0, row1);        \
  __m256d tmp2 = _mm256_unpacklo_pd(row2, row1);        \
  __m256d tmp3 = _mm256_unpackhi_pd(row2, row1);        \
  row0 = _mm256_permute2f128_pd(tmp0, tmp2, 0x20);      \
  row1 = _mm256_permute2f128_pd(tmp1, tmp3, 0x20);      \
  row2 = _mm256_permute2f128_pd(tmp0, tmp2, 0x31);      \
}
//
int ElastOrtho3D::ElemLinear( Elem* ){ return 1; };//FIXME
int ElastOrtho3D::ElemJacobi( Elem* ){ return 1; };//FIXME
int ElastOrtho3D::ElemStiff( Elem* ){ return 1; };//FIXME
//
int ElastOrtho3D::Setup( Elem* E ){
  JacRot( E );
  JacT  ( E );
  const uint jacs_n = E->elip_jacs.size()/E->elem_n/ 10 ;
  const uint intp_n = E->gaus_n;
  this->tens_flop = uint(E->elem_n) * intp_n *( uint(E->elem_conn_n)* (36+18+3) + 2*54 + 27 );
  this->tens_band = uint(E->elem_n) *( sizeof(FLOAT_PHYS)*(3*uint(E->elem_conn_n)*3+ jacs_n*10) +sizeof(INT_MESH)*uint(E->elem_conn_n) );
  this->stif_flop = uint(E->elem_n) * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) *( sizeof(FLOAT_PHYS)* 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) -1+3) +sizeof(INT_MESH) *uint(E->elem_conn_n) );
  return 0;
};
int ElastOrtho3D::ElemLinear( Elem* E, RESTRICT Phys::vals &sys_f, const RESTRICT Phys::vals &sys_u ){
  //FIXME Cleanup local variables.
  const int ndof = 3;//this->node_d
  const int Nf   = 3;// this->node_d DOF/node
  const int Nj   =10;//,d2=9;//mesh_d*mesh_d;
  const INT_MESH elem_n = E->elem_n;
  const int   Nc = E->elem_conn_n;// Number of Nodes/Element
  const int   Ne = ndof*Nc;
  const int intp_n = int(E->gaus_n);
  //
  INT_MESH e0=0, ee=elem_n;
  if(E->do_halo==true){ e0=0; ee=E->halo_elem_n;
  }else{ e0=E->halo_elem_n; ee=elem_n; };
  //
#if VERB_MAX>11
  printf("Elems:%i, IntPts:%i, Nodes/elem:%i\n",
      (int)elem_n,(int)intp_n,(int)Nc);
#endif
  FLOAT_PHYS dw;
  FLOAT_MESH __attribute__((aligned(64))) jac[Nj];//, det;
  FLOAT_PHYS __attribute__((aligned(64))) G[Ne+1], u[Ne+1],f[Ne+1];
  FLOAT_PHYS __attribute__((aligned(64))) H[9+1], S[9], A[9+1];
  //
  FLOAT_PHYS __attribute__((aligned(64))) intp_shpg[intp_n*Ne];
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );// local copy
  FLOAT_PHYS __attribute__((aligned(64))) wgt[intp_n];
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  FLOAT_PHYS __attribute__((aligned(64))) C[this->mtrl_matc.size()];
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[this->mtrl_matc.size()], C );
#if 1
  __m256d c0,c1,c2,c3;
  c0 = _mm256_set_pd(0.,C[5],C[3],C[0]); c1 = _mm256_set_pd(0.,C[4],C[1],C[3]); c2 = _mm256_set_pd(0.,C[2],C[4],C[5]); c3 = _mm256_set_pd(0.,C[7],C[8],C[6]);
#endif

  const FLOAT_PHYS __attribute__((aligned(64))) R[9+1] = {
    mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
    mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
    mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8],0.0};
#if 0
// Slower without this variable even though it isn't used (44->42) UNTIL I added the aligned decorations above!  Indicates other padding may help.
  const __attribute__((aligned(64))) FLOAT_PHYS Rt[9+1] = {
    mtrl_rotc[0],mtrl_rotc[3],mtrl_rotc[6],
    mtrl_rotc[1],mtrl_rotc[4],mtrl_rotc[7],
    mtrl_rotc[2],mtrl_rotc[5],mtrl_rotc[8],0.0};
#endif
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  }; printf("\n");
#endif
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &sys_u[0];
  if(e0<ee){
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
    const   INT_MESH* RESTRICT c = &Econn[Nc*e0];
    for (int i=0; i<Nc; i++){
      std::memcpy( &    u[ndof*i], &sysu[c[i]*ndof], sizeof(FLOAT_SOLV)*ndof );
    };
  };
  //bool fetch_next=false;

  for(INT_MESH ie=e0;ie<ee;ie++){

#if 1
    __m256d f0,f1,f2,f3,f4,f5,f6,f7,f8,f9;
  const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
  f0 = _mm256_loadu_pd(&sys_f[3*conn[0]]); f1 = _mm256_loadu_pd(&sys_f[3*conn[1]]); f2 = _mm256_loadu_pd(&sys_f[3*conn[2]]);
  f3 = _mm256_loadu_pd(&sys_f[3*conn[3]]); f4 = _mm256_loadu_pd(&sys_f[3*conn[4]]); f5 = _mm256_loadu_pd(&sys_f[3*conn[5]]);
  f6 = _mm256_loadu_pd(&sys_f[3*conn[6]]); f7 = _mm256_loadu_pd(&sys_f[3*conn[7]]); f8 = _mm256_loadu_pd(&sys_f[3*conn[8]]);
  f9 = _mm256_loadu_pd(&sys_f[3*conn[9]]);
  //f0 = _mm256_setzero_pd(); f1 = _mm256_setzero_pd(); f2 = _mm256_setzero_pd(); f3 = _mm256_setzero_pd(); f4 = _mm256_setzero_pd();
  //f5 = _mm256_setzero_pd(); f6 = _mm256_setzero_pd(); f7 = _mm256_setzero_pd(); f8 = _mm256_setzero_pd(); f9 = _mm256_setzero_pd();
#else
  for(int i=0;i<Ne;i++){ f[i]=0.0; };
#endif
#if 1
  __m256d j0,j1,j2;
  j0 = _mm256_loadu_pd(&jac[0]);  // j0 = [j3 j2 j1 j0]
  j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
  j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]
#else
  __m512d j0,j1,j2;
  j0 = _mm512_broadcast_f64x4(_mm256_loadu_pd(&jac[0]));
  j1 = _mm512_broadcast_f64x4(_mm256_loadu_pd(&jac[3]));
  j2 = _mm512_broadcast_f64x4(_mm256_loadu_pd(&jac[6]));
#endif
  //_MM256_TRANSPOSE3_PD(j0,j1,j2); //    [   j6 j3 j0]
  //    [   j7 j4 j1]
  //    [   j8 j5 j2]

  for(int ip=0; ip<intp_n; ip++){
#if 1
#if 1
#if 1
#if 0
    __m512d u0,u1,u2,u3,u4,u5,g0,g1,g2;
    __m512d is0,is1,is2,is3,is4,is5;
    __m512d a036zmm, a147zmm, a258zmm;
    __m256d a036, a147, a258;
    double * RESTRICT isp = &intp_shpg[ip*Ne];

    is0= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[0])), _mm256_set1_pd(isp[3]),0x1);
    is1= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[1])), _mm256_set1_pd(isp[4]),0x1);
    is2= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[2])), _mm256_set1_pd(isp[5]),0x1);

    u0 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[0])), _mm256_set1_pd(u[3]),0x1);
    u1 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[1])), _mm256_set1_pd(u[4]),0x1);
    u2 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[2])), _mm256_set1_pd(u[5]),0x1);

    g0 = _mm512_add_pd(_mm512_mul_pd(j0,is0), _mm512_add_pd(_mm512_mul_pd(j1,is1),_mm512_mul_pd(j2,is2)));
    a036zmm = _mm512_mul_pd(g0,u0); a147zmm = _mm512_mul_pd(g0,u1); a258zmm = _mm512_mul_pd(g0,u2);
    _mm256_storeu_pd(&G[0], _mm512_castpd512_pd256(g0));
    _mm256_storeu_pd(&G[3], _mm512_extractf64x4_pd(g0,0x1));

    is3= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[6])), _mm256_set1_pd(isp[ 9]),0x1);
    is4= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[7])), _mm256_set1_pd(isp[10]),0x1);
    is5= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[8])), _mm256_set1_pd(isp[11]),0x1);

    u3 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[6])), _mm256_set1_pd(u[ 9]),0x1);
    u4 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[7])), _mm256_set1_pd(u[10]),0x1);
    u5 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[8])), _mm256_set1_pd(u[11]),0x1);

    g1 = _mm512_add_pd(_mm512_mul_pd(j0,is3), _mm512_add_pd(_mm512_mul_pd(j1,is4),_mm512_mul_pd(j2,is5)));
    a036zmm = _mm512_add_pd(a036zmm, _mm512_mul_pd(g0,u3));
    a147zmm = _mm512_add_pd(a147zmm, _mm512_mul_pd(g1,u4));
    a258zmm = _mm512_add_pd(a258zmm, _mm512_mul_pd(g1,u5));
    _mm256_storeu_pd(&G[6], _mm512_castpd512_pd256(g1));
    _mm256_storeu_pd(&G[9], _mm512_extractf64x4_pd(g1,0x1));

    is0= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[12])), _mm256_set1_pd(isp[15]),0x1);
    is1= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[13])), _mm256_set1_pd(isp[16]),0x1);
    is2= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[14])), _mm256_set1_pd(isp[17]),0x1);

    u0 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[12])), _mm256_set1_pd(u[15]),0x1);
    u1 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[13])), _mm256_set1_pd(u[16]),0x1);
    u2 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[14])), _mm256_set1_pd(u[17]),0x1);

    g0 = _mm512_add_pd(_mm512_mul_pd(j0,is0), _mm512_add_pd(_mm512_mul_pd(j1,is1),_mm512_mul_pd(j2,is2)));
    a036zmm = _mm512_add_pd(a036zmm, _mm512_mul_pd(g0,u0));
    a147zmm = _mm512_add_pd(a147zmm, _mm512_mul_pd(g0,u1));
    a258zmm = _mm512_add_pd(a258zmm, _mm512_mul_pd(g0,u2));
    _mm256_storeu_pd(&G[12], _mm512_castpd512_pd256(g0));
    _mm256_storeu_pd(&G[15], _mm512_extractf64x4_pd(g0,0x1));

    is3= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[18])), _mm256_set1_pd(isp[21]),0x1);
    is4= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[19])), _mm256_set1_pd(isp[22]),0x1);
    is5= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[20])), _mm256_set1_pd(isp[23]),0x1);

    u3 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[18])), _mm256_set1_pd(u[21]),0x1);
    u4 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[19])), _mm256_set1_pd(u[22]),0x1);
    u5 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[20])), _mm256_set1_pd(u[23]),0x1);

    g1 = _mm512_add_pd(_mm512_mul_pd(j0,is3), _mm512_add_pd(_mm512_mul_pd(j1,is4),_mm512_mul_pd(j2,is5)));
    a036zmm = _mm512_add_pd(a036zmm, _mm512_mul_pd(g0,u3));
    a147zmm = _mm512_add_pd(a147zmm, _mm512_mul_pd(g1,u4));
    a258zmm = _mm512_add_pd(a258zmm, _mm512_mul_pd(g1,u5));
    _mm256_storeu_pd(&G[18], _mm512_castpd512_pd256(g1));
    _mm256_storeu_pd(&G[21], _mm512_extractf64x4_pd(g1,0x1));

    is0= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[24])), _mm256_set1_pd(isp[27]),0x1);
    is1= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[25])), _mm256_set1_pd(isp[28]),0x1);
    is2= _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(isp[26])), _mm256_set1_pd(isp[29]),0x1);

    u0 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[24])), _mm256_set1_pd(u[27]),0x1);
    u1 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[25])), _mm256_set1_pd(u[28]),0x1);
    u2 = _mm512_insertf64x4(_mm512_castpd256_pd512(_mm256_set1_pd(u[26])), _mm256_set1_pd(u[29]),0x1);

    g0 = _mm512_add_pd(_mm512_mul_pd(j0,is0), _mm512_add_pd(_mm512_mul_pd(j1,is1),_mm512_mul_pd(j2,is2)));
    a036zmm = _mm512_add_pd(a036zmm, _mm512_mul_pd(g0,u0));
    a147zmm = _mm512_add_pd(a147zmm, _mm512_mul_pd(g0,u1));
    a258zmm = _mm512_add_pd(a258zmm, _mm512_mul_pd(g0,u2));
    _mm256_storeu_pd(&G[24], _mm512_castpd512_pd256(g0));
    _mm256_storeu_pd(&G[27], _mm512_extractf64x4_pd(g0,0x1));

    a036 = _mm256_add_pd(_mm512_castpd512_pd256(a036zmm), _mm512_extractf64x4_pd(a036zmm,0x1));
    a147 = _mm256_add_pd(_mm512_castpd512_pd256(a147zmm), _mm512_extractf64x4_pd(a147zmm,0x1));
    a258 = _mm256_add_pd(_mm512_castpd512_pd256(a258zmm), _mm512_extractf64x4_pd(a258zmm,0x1));

    _MM256_TRANSPOSE3_PD(a036,a147,a258);
    _mm256_storeu_pd(&A[0],a036); // [?? a2 a1 a0]
    _mm256_storeu_pd(&A[3],a147); // [?? a5 a4 a3]
    _mm256_storeu_pd(&A[6],a258); // [?? a8 a7 a6]
#else
    __m256d u0,u1,u2,u3,u4,u5,g0,g1,g2;
    __m256d is0,is1,is2,is3,is4,is5;
    __m256d a036, a147, a258;
    double * RESTRICT isp = &intp_shpg[ip*Ne];

    is0= _mm256_set1_pd(isp[0]); is1= _mm256_set1_pd(isp[1]); is2= _mm256_set1_pd(isp[2]);
    u0 = _mm256_set1_pd(  u[0]); u1 = _mm256_set1_pd(  u[1]); u2 = _mm256_set1_pd(  u[2]);
    g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0), _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
    a036= _mm256_mul_pd(g0,u0); a147 = _mm256_mul_pd(g0,u1); a258 = _mm256_mul_pd(g0,u2);
         _mm256_storeu_pd(&G[0],g0);

    is3= _mm256_set1_pd(isp[3]); is4= _mm256_set1_pd(isp[4]); is5= _mm256_set1_pd(isp[5]);
    u3 = _mm256_set1_pd(  u[3]); u4 = _mm256_set1_pd(  u[4]); u5 = _mm256_set1_pd(  u[5]);
    g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3), _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
         _mm256_storeu_pd(&G[3],g1);

    is0= _mm256_set1_pd(isp[6]); is1= _mm256_set1_pd(isp[7]); is2= _mm256_set1_pd(isp[8]);
    u0 = _mm256_set1_pd(  u[6]); u1 = _mm256_set1_pd(  u[7]); u2 = _mm256_set1_pd(  u[8]);
    g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0), _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
         _mm256_storeu_pd(&G[6],g0);

    is3= _mm256_set1_pd(isp[9]); is4= _mm256_set1_pd(isp[10]); is5= _mm256_set1_pd(isp[11]);
    u3 = _mm256_set1_pd(  u[9]); u4 = _mm256_set1_pd(  u[10]); u5 = _mm256_set1_pd(  u[11]);
    g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3), _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
         _mm256_storeu_pd(&G[9],g1);

    is0= _mm256_set1_pd(isp[12]); is1= _mm256_set1_pd(isp[13]); is2= _mm256_set1_pd(isp[14]);
    u0 = _mm256_set1_pd(  u[12]); u1 = _mm256_set1_pd(  u[13]); u2 = _mm256_set1_pd(  u[14]);
    g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0), _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
         _mm256_storeu_pd(&G[12],g0);

    is3= _mm256_set1_pd(isp[15]); is4= _mm256_set1_pd(isp[16]); is5= _mm256_set1_pd(isp[17]);
    u3 = _mm256_set1_pd(  u[15]); u4 = _mm256_set1_pd(  u[16]); u5 = _mm256_set1_pd(  u[17]);
    g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3), _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
         _mm256_storeu_pd(&G[15],g1);

    is0= _mm256_set1_pd(isp[18]); is1= _mm256_set1_pd(isp[19]); is2= _mm256_set1_pd(isp[20]);
    u0 = _mm256_set1_pd(  u[18]); u1 = _mm256_set1_pd(  u[19]); u2 = _mm256_set1_pd(  u[20]);
    g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0), _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
         _mm256_storeu_pd(&G[18],g0);

    is3= _mm256_set1_pd(isp[21]); is4= _mm256_set1_pd(isp[22]); is5= _mm256_set1_pd(isp[23]);
    u3 = _mm256_set1_pd(  u[21]); u4 = _mm256_set1_pd(  u[22]); u5 = _mm256_set1_pd(  u[23]);
    g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3), _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
         _mm256_storeu_pd(&G[21],g1);

    is0= _mm256_set1_pd(isp[24]); is1= _mm256_set1_pd(isp[25]); is2= _mm256_set1_pd(isp[26]);
    u0 = _mm256_set1_pd(  u[24]); u1 = _mm256_set1_pd(  u[25]); u2 = _mm256_set1_pd(  u[26]);
    g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0), _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
         _mm256_storeu_pd(&G[24],g0);

    is3= _mm256_set1_pd(isp[27]); is4= _mm256_set1_pd(isp[28]); is5= _mm256_set1_pd(isp[29]);
    u3 = _mm256_set1_pd(  u[27]); u4 = _mm256_set1_pd(  u[28]); u5 = _mm256_set1_pd(  u[29]);
    g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3), _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
         _mm256_storeu_pd(&G[27],g1);

    __m256d r0,r1,r2,r3,r4,r5,r6,r7,r8,h036,h147,h258;
    r0 = _mm256_set1_pd(R[0]); r1 = _mm256_set1_pd(R[1]); r2 = _mm256_set1_pd(R[2]);
    r3 = _mm256_set1_pd(R[3]); r4 = _mm256_set1_pd(R[4]); r5 = _mm256_set1_pd(R[5]);
    r6 = _mm256_set1_pd(R[6]); r7 = _mm256_set1_pd(R[7]); r8 = _mm256_set1_pd(R[8]);
    h036 = _mm256_add_pd(_mm256_mul_pd(r0,a036), _mm256_add_pd(_mm256_mul_pd(r1,a147),_mm256_mul_pd(r2,a258)));
    h147 = _mm256_add_pd(_mm256_mul_pd(r3,a036), _mm256_add_pd(_mm256_mul_pd(r4,a147),_mm256_mul_pd(r5,a258)));
    h258 = _mm256_add_pd(_mm256_mul_pd(r6,a036), _mm256_add_pd(_mm256_mul_pd(r7,a147),_mm256_mul_pd(r8,a258)));

    // Take advantage of the fact that the pattern of usage is invariant with respect to transpose _MM256_TRANSPOSE3_PD(h036,h147,h258);
    _mm256_storeu_pd(&H[0],h036);
    _mm256_storeu_pd(&H[3],h147);
    _mm256_storeu_pd(&H[6],h258);
#endif
#else
    __m256d s0,s1,s2,is0,is1,u0,u1,a01,a31,a61,g0,g1,g2;
    __m256d s3,s4,s5,is2,is3,u2,u3,a02,a32,a62,g3,g4,g5;
    double * RESTRICT isp = &intp_shpg[ip*Ne];
    a01= _mm256_setzero_pd(); a31 = _mm256_setzero_pd(); a61 = _mm256_setzero_pd();
    a02= _mm256_setzero_pd(); a32 = _mm256_setzero_pd(); a62 = _mm256_setzero_pd();
    //j0 = _mm256_loadu_pd(&jac[0]);  // j0 = [j3 j2 j1 j0]
    //j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
    //j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]

    is0= _mm256_loadu_pd(isp) ; u0 = _mm256_loadu_pd(&u[0]);
    s0 = _mm256_mul_pd(j0,is0); s1 = _mm256_mul_pd(j1,is0); s2 = _mm256_mul_pd(j2,is0);
    _MM256_TRANSPOSE3_PD(s0,s1,s2);
    s0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
    _mm256_storeu_pd(&G[0],s0);
    //g0 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s0)); g1 = _mm256_set1_pd(G[1]); g2 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s0,0x1));
    g0 = _mm256_set1_pd(G[0]); g1 = _mm256_set1_pd(G[1]); g2 = _mm256_set1_pd(G[2]);
    a01= _mm256_add_pd(a01, _mm256_mul_pd(u0,g0));
    a31= _mm256_add_pd(a31, _mm256_mul_pd(u0,g1));
    a61= _mm256_add_pd(a61, _mm256_mul_pd(u0,g2));

    is1= _mm256_loadu_pd(isp+3) ; u1 = _mm256_loadu_pd(&u[3]);
    s3 = _mm256_mul_pd(j0,is1); s4 = _mm256_mul_pd(j1,is1); s5 = _mm256_mul_pd(j2,is1);
    _MM256_TRANSPOSE3_PD(s3,s4,s5);
    s3 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
    _mm256_storeu_pd(&G[3],s3);
    //g3 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s3)); g4 = _mm256_set1_pd(G[4]); g5 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s3,0x1));
    g3 = _mm256_set1_pd(G[3]); g4 = _mm256_set1_pd(G[4]); g5 = _mm256_set1_pd(G[5]);
    a02= _mm256_add_pd(a02, _mm256_mul_pd(u1,g3));
    a32= _mm256_add_pd(a32, _mm256_mul_pd(u1,g4));
    a62= _mm256_add_pd(a62, _mm256_mul_pd(u1,g5));

    is2= _mm256_loadu_pd(isp+6) ; u2 = _mm256_loadu_pd(&u[6]);
    s0 = _mm256_mul_pd(j0,is2); s1 = _mm256_mul_pd(j1,is2); s2 = _mm256_mul_pd(j2,is2);
    _MM256_TRANSPOSE3_PD(s0,s1,s2);
    s0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
    _mm256_storeu_pd(&G[6],s0);
    //g0 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s0)); g1 = _mm256_set1_pd(G[7]); g2 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s0,0x1));
    g0 = _mm256_set1_pd(G[6]); g1 = _mm256_set1_pd(G[7]); g2 = _mm256_set1_pd(G[8]);
    a01= _mm256_add_pd(a01, _mm256_mul_pd(u2,g0));
    a31= _mm256_add_pd(a31, _mm256_mul_pd(u2,g1));
    a61= _mm256_add_pd(a61, _mm256_mul_pd(u2,g2));

    is3= _mm256_loadu_pd(isp+9) ; u3 = _mm256_loadu_pd(&u[9]);
    s3 = _mm256_mul_pd(j0,is3); s4 = _mm256_mul_pd(j1,is3); s5 = _mm256_mul_pd(j2,is3);
    _MM256_TRANSPOSE3_PD(s3,s4,s5);
    s3 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
    _mm256_storeu_pd(&G[9],s3);
    //g3 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s3)); g4 = _mm256_set1_pd(G[10]); g5 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s3,0x1));
    g3 = _mm256_set1_pd(G[9]); g4 = _mm256_set1_pd(G[10]); g5 = _mm256_set1_pd(G[11]);
    a02= _mm256_add_pd(a02, _mm256_mul_pd(u3,g3));
    a32= _mm256_add_pd(a32, _mm256_mul_pd(u3,g4));
    a62= _mm256_add_pd(a62, _mm256_mul_pd(u3,g5));


    is0= _mm256_loadu_pd(isp+12) ; u0 = _mm256_loadu_pd(&u[12]);
    s0 = _mm256_mul_pd(j0,is0); s1 = _mm256_mul_pd(j1,is0); s2 = _mm256_mul_pd(j2,is0);
    _MM256_TRANSPOSE3_PD(s0,s1,s2);
    s0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
    _mm256_storeu_pd(&G[12],s0);
    //g0 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s0)); g1 = _mm256_set1_pd(G[13]); g2 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s0,0x1));
    g0 = _mm256_set1_pd(G[12]); g1 = _mm256_set1_pd(G[13]); g2 = _mm256_set1_pd(G[14]);
    a01= _mm256_add_pd(a01, _mm256_mul_pd(u0,g0));
    a31= _mm256_add_pd(a31, _mm256_mul_pd(u0,g1));
    a61= _mm256_add_pd(a61, _mm256_mul_pd(u0,g2));

    is1= _mm256_loadu_pd(isp+15) ; u1 = _mm256_loadu_pd(&u[15]);
    s3 = _mm256_mul_pd(j0,is1); s4 = _mm256_mul_pd(j1,is1); s5 = _mm256_mul_pd(j2,is1);
    _MM256_TRANSPOSE3_PD(s3,s4,s5);
    s3 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
    _mm256_storeu_pd(&G[15],s3);
    //g3 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s3)); g4 = _mm256_set1_pd(G[16]); g5 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s3,0x1));
    g3 = _mm256_set1_pd(G[15]); g4 = _mm256_set1_pd(G[16]); g5 = _mm256_set1_pd(G[17]);
    a02= _mm256_add_pd(a02, _mm256_mul_pd(u1,g3));
    a32= _mm256_add_pd(a32, _mm256_mul_pd(u1,g4));
    a62= _mm256_add_pd(a62, _mm256_mul_pd(u1,g5));

    is2= _mm256_loadu_pd(isp+18) ; u2 = _mm256_loadu_pd(&u[18]);
    s0 = _mm256_mul_pd(j0,is2); s1 = _mm256_mul_pd(j1,is2); s2 = _mm256_mul_pd(j2,is2);
    _MM256_TRANSPOSE3_PD(s0,s1,s2);
    s0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
    _mm256_storeu_pd(&G[18],s0);
    //g0 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s0)); g1 = _mm256_set1_pd(G[19]); g2 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s0,0x1));
    g0 = _mm256_set1_pd(G[18]); g1 = _mm256_set1_pd(G[19]); g2 = _mm256_set1_pd(G[20]);
    a01= _mm256_add_pd(a01, _mm256_mul_pd(u2,g0));
    a31= _mm256_add_pd(a31, _mm256_mul_pd(u2,g1));
    a61= _mm256_add_pd(a61, _mm256_mul_pd(u2,g2));

    is3= _mm256_loadu_pd(isp+21) ; u3 = _mm256_loadu_pd(&u[21]);
    s3 = _mm256_mul_pd(j0,is3); s4 = _mm256_mul_pd(j1,is3); s5 = _mm256_mul_pd(j2,is3);
    _MM256_TRANSPOSE3_PD(s3,s4,s5);
    s3 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
    _mm256_storeu_pd(&G[21],s3);
    //g3 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s3)); g4 = _mm256_set1_pd(G[22]); g5 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s3,0x1));
    g3 = _mm256_set1_pd(G[21]); g4 = _mm256_set1_pd(G[22]); g5 = _mm256_set1_pd(G[23]);
    a02= _mm256_add_pd(a02, _mm256_mul_pd(u3,g3));
    a32= _mm256_add_pd(a32, _mm256_mul_pd(u3,g4));
    a62= _mm256_add_pd(a62, _mm256_mul_pd(u3,g5));


    is0= _mm256_loadu_pd(isp+24) ; u0 = _mm256_loadu_pd(&u[24]);
    s0 = _mm256_mul_pd(j0,is0); s1 = _mm256_mul_pd(j1,is0); s2 = _mm256_mul_pd(j2,is0);
    _MM256_TRANSPOSE3_PD(s0,s1,s2);
    s0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
    _mm256_storeu_pd(&G[24],s0);
    //g0 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s0)); g1 = _mm256_set1_pd(G[25]); g2 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s0,0x1));
    g0 = _mm256_set1_pd(G[24]); g1 = _mm256_set1_pd(G[25]); g2 = _mm256_set1_pd(G[26]);
    a01= _mm256_add_pd(a01, _mm256_mul_pd(u0,g0));
    a31= _mm256_add_pd(a31, _mm256_mul_pd(u0,g1));
    a61= _mm256_add_pd(a61, _mm256_mul_pd(u0,g2));

    is1= _mm256_loadu_pd(isp+27) ; u1 = _mm256_loadu_pd(&u[27]);
    s3 = _mm256_mul_pd(j0,is1); s4 = _mm256_mul_pd(j1,is1); s5 = _mm256_mul_pd(j2,is1);
    _MM256_TRANSPOSE3_PD(s3,s4,s5);
    s3 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
    _mm256_storeu_pd(&G[27],s3);
    //g3 = _mm256_broadcastsd_pd(_mm256_castpd256_pd128(s3)); g4 = _mm256_set1_pd(G[28]); g5 = _mm256_broadcastsd_pd(_mm256_extractf128_pd(s3,0x1));
    g3 = _mm256_set1_pd(G[27]); g4 = _mm256_set1_pd(G[28]); g5 = _mm256_set1_pd(G[29]);
    a02= _mm256_add_pd(a01, _mm256_add_pd(a02, _mm256_mul_pd(u1,g3)));
    a32= _mm256_add_pd(a31, _mm256_add_pd(a32, _mm256_mul_pd(u1,g4)));
    a62= _mm256_add_pd(a61, _mm256_add_pd(a62, _mm256_mul_pd(u1,g5)));

    _mm256_storeu_pd(&A[0],a02);
    _mm256_storeu_pd(&A[3],a32);
    _mm256_storeu_pd(&A[6],a62);
#endif
#else
    for(int k=0; k<Nc; k++){
      double * RESTRICT Gp = &G[3*k];
      double * RESTRICT isp = &intp_shpg[ip*Ne+ 3*k];
      double * RESTRICT up = &u[ndof*k];

      is0= _mm256_loadu_pd(isp) ; u0 = _mm256_loadu_pd(up);
      s0 = _mm256_mul_pd(j0,is0); s1 = _mm256_mul_pd(j1,is0); s2 = _mm256_mul_pd(j2,is0);
      _MM256_TRANSPOSE3_PD(s0,s1,s2);
      s0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
      _mm256_storeu_pd(Gp,s0);
      g0 = _mm256_set1_pd(Gp[0]); g1 = _mm256_set1_pd(Gp[1]); g2 = _mm256_set1_pd(Gp[2]);
      a0 = _mm256_add_pd(a0, _mm256_mul_pd(u0,g0));
      a3 = _mm256_add_pd(a3, _mm256_mul_pd(u0,g1));
      a6 = _mm256_add_pd(a6, _mm256_mul_pd(u0,g2));
    };//------------------------------------------------- N*3*6*2 = 36*N FLOP
    _mm256_storeu_pd(&A[0],a0);
    _mm256_storeu_pd(&A[3],a3);
    _mm256_storeu_pd(&A[6],a6);
#endif
#else
#if 0
#if 1
    G[0] = (jac[0]*intp_shpg[ip*Ne+0]) + (jac[1]*intp_shpg[ip*Ne+1]) + (jac[2]*intp_shpg[ip*Ne+2]);
    G[1] = (jac[3]*intp_shpg[ip*Ne+0]) + (jac[4]*intp_shpg[ip*Ne+1]) + (jac[5]*intp_shpg[ip*Ne+2]);
    G[2] = (jac[6]*intp_shpg[ip*Ne+0]) + (jac[7]*intp_shpg[ip*Ne+1]) + (jac[8]*intp_shpg[ip*Ne+2]);
    A[0] = G[0]*u[0]; A[3] = G[1]*u[0]; A[6] = G[2]*u[0];
    A[1] = G[0]*u[1]; A[4] = G[1]*u[1]; A[7] = G[2]*u[1];
    A[2] = G[0]*u[2]; A[5] = G[1]*u[2]; A[8] = G[2]*u[2];

    G[3] = (jac[0]*intp_shpg[ip*Ne+3]) + (jac[1]*intp_shpg[ip*Ne+4]) + (jac[2]*intp_shpg[ip*Ne+5]);
    G[4] = (jac[3]*intp_shpg[ip*Ne+3]) + (jac[4]*intp_shpg[ip*Ne+4]) + (jac[5]*intp_shpg[ip*Ne+5]);
    G[5] = (jac[6]*intp_shpg[ip*Ne+3]) + (jac[7]*intp_shpg[ip*Ne+4]) + (jac[8]*intp_shpg[ip*Ne+5]);
    A[0] += G[3]*u[3]; A[3] += G[4]*u[3]; A[6] += G[5]*u[3];
    A[1] += G[3]*u[4]; A[4] += G[4]*u[4]; A[7] += G[5]*u[4];
    A[2] += G[3]*u[5]; A[5] += G[4]*u[5]; A[8] += G[5]*u[5];

    G[6] = (jac[0]*intp_shpg[ip*Ne+6]) + (jac[1]*intp_shpg[ip*Ne+7]) + (jac[2]*intp_shpg[ip*Ne+8]);
    G[7] = (jac[3]*intp_shpg[ip*Ne+6]) + (jac[4]*intp_shpg[ip*Ne+7]) + (jac[5]*intp_shpg[ip*Ne+8]);
    G[8] = (jac[6]*intp_shpg[ip*Ne+6]) + (jac[7]*intp_shpg[ip*Ne+7]) + (jac[8]*intp_shpg[ip*Ne+8]);
    A[0] += G[6]*u[6]; A[3] += G[7]*u[6]; A[6] += G[8]*u[6];
    A[1] += G[6]*u[7]; A[4] += G[7]*u[7]; A[7] += G[8]*u[7];
    A[2] += G[6]*u[8]; A[5] += G[7]*u[8]; A[8] += G[8]*u[8];

    G[ 9] = (jac[0]*intp_shpg[ip*Ne+9]) + (jac[1]*intp_shpg[ip*Ne+10]) + (jac[2]*intp_shpg[ip*Ne+11]);
    G[10] = (jac[3]*intp_shpg[ip*Ne+9]) + (jac[4]*intp_shpg[ip*Ne+10]) + (jac[5]*intp_shpg[ip*Ne+11]);
    G[11] = (jac[6]*intp_shpg[ip*Ne+9]) + (jac[7]*intp_shpg[ip*Ne+10]) + (jac[8]*intp_shpg[ip*Ne+11]);
    A[0] += G[9]*u[ 9]; A[3] += G[10]*u[ 9]; A[6] += G[11]*u[ 9];
    A[1] += G[9]*u[10]; A[4] += G[10]*u[10]; A[7] += G[11]*u[10];
    A[2] += G[9]*u[11]; A[5] += G[10]*u[11]; A[8] += G[11]*u[11];

    G[12] = (jac[0]*intp_shpg[ip*Ne+12]) + (jac[1]*intp_shpg[ip*Ne+13]) + (jac[2]*intp_shpg[ip*Ne+14]);
    G[13] = (jac[3]*intp_shpg[ip*Ne+12]) + (jac[4]*intp_shpg[ip*Ne+13]) + (jac[5]*intp_shpg[ip*Ne+14]);
    G[14] = (jac[6]*intp_shpg[ip*Ne+12]) + (jac[7]*intp_shpg[ip*Ne+13]) + (jac[8]*intp_shpg[ip*Ne+14]);
    A[0] += G[12]*u[12]; A[3] += G[13]*u[12]; A[6] += G[14]*u[12];
    A[1] += G[12]*u[13]; A[4] += G[13]*u[13]; A[7] += G[14]*u[13];
    A[2] += G[12]*u[14]; A[5] += G[13]*u[14]; A[8] += G[14]*u[14];

    G[15] = (jac[0]*intp_shpg[ip*Ne+15]) + (jac[1]*intp_shpg[ip*Ne+16]) + (jac[2]*intp_shpg[ip*Ne+17]);
    G[16] = (jac[3]*intp_shpg[ip*Ne+15]) + (jac[4]*intp_shpg[ip*Ne+16]) + (jac[5]*intp_shpg[ip*Ne+17]);
    G[17] = (jac[6]*intp_shpg[ip*Ne+15]) + (jac[7]*intp_shpg[ip*Ne+16]) + (jac[8]*intp_shpg[ip*Ne+17]);
    A[0] += G[15]*u[15]; A[3] += G[16]*u[15]; A[6] += G[17]*u[15];
    A[1] += G[15]*u[16]; A[4] += G[16]*u[16]; A[7] += G[17]*u[16];
    A[2] += G[15]*u[17]; A[5] += G[16]*u[17]; A[8] += G[17]*u[17];

    G[18] = (jac[0]*intp_shpg[ip*Ne+18]) + (jac[1]*intp_shpg[ip*Ne+19]) + (jac[2]*intp_shpg[ip*Ne+20]);
    G[19] = (jac[3]*intp_shpg[ip*Ne+18]) + (jac[4]*intp_shpg[ip*Ne+19]) + (jac[5]*intp_shpg[ip*Ne+20]);
    G[20] = (jac[6]*intp_shpg[ip*Ne+18]) + (jac[7]*intp_shpg[ip*Ne+19]) + (jac[8]*intp_shpg[ip*Ne+20]);
    A[0] += G[18]*u[18]; A[3] += G[19]*u[18]; A[6] += G[20]*u[18];
    A[1] += G[18]*u[19]; A[4] += G[19]*u[19]; A[7] += G[20]*u[19];
    A[2] += G[18]*u[20]; A[5] += G[19]*u[20]; A[8] += G[20]*u[20];

    G[21] = (jac[0]*intp_shpg[ip*Ne+21]) + (jac[1]*intp_shpg[ip*Ne+22]) + (jac[2]*intp_shpg[ip*Ne+23]);
    G[22] = (jac[3]*intp_shpg[ip*Ne+21]) + (jac[4]*intp_shpg[ip*Ne+22]) + (jac[5]*intp_shpg[ip*Ne+23]);
    G[23] = (jac[6]*intp_shpg[ip*Ne+21]) + (jac[7]*intp_shpg[ip*Ne+22]) + (jac[8]*intp_shpg[ip*Ne+23]);
    A[0] += G[21]*u[21]; A[3] += G[22]*u[21]; A[6] += G[23]*u[21];
    A[1] += G[21]*u[22]; A[4] += G[22]*u[22]; A[7] += G[23]*u[22];
    A[2] += G[21]*u[23]; A[5] += G[22]*u[23]; A[8] += G[23]*u[23];

    G[24] = (jac[0]*intp_shpg[ip*Ne+24]) + (jac[1]*intp_shpg[ip*Ne+25]) + (jac[2]*intp_shpg[ip*Ne+26]);
    G[25] = (jac[3]*intp_shpg[ip*Ne+24]) + (jac[4]*intp_shpg[ip*Ne+25]) + (jac[5]*intp_shpg[ip*Ne+26]);
    G[26] = (jac[6]*intp_shpg[ip*Ne+24]) + (jac[7]*intp_shpg[ip*Ne+25]) + (jac[8]*intp_shpg[ip*Ne+26]);
    A[0] += G[24]*u[24]; A[3] += G[25]*u[24]; A[6] += G[26]*u[24];
    A[1] += G[24]*u[25]; A[4] += G[25]*u[25]; A[7] += G[26]*u[25];
    A[2] += G[24]*u[26]; A[5] += G[25]*u[26]; A[8] += G[26]*u[26];

    G[27] = (jac[0]*intp_shpg[ip*Ne+27]) + (jac[1]*intp_shpg[ip*Ne+28]) + (jac[2]*intp_shpg[ip*Ne+29]);
    G[28] = (jac[3]*intp_shpg[ip*Ne+27]) + (jac[4]*intp_shpg[ip*Ne+28]) + (jac[5]*intp_shpg[ip*Ne+29]);
    G[29] = (jac[6]*intp_shpg[ip*Ne+27]) + (jac[7]*intp_shpg[ip*Ne+28]) + (jac[8]*intp_shpg[ip*Ne+29]);
    A[0] += G[27]*u[27]; A[3] += G[28]*u[27]; A[6] += G[29]*u[27];
    A[1] += G[27]*u[28]; A[4] += G[28]*u[28]; A[7] += G[29]*u[28];
    A[2] += G[27]*u[29]; A[5] += G[28]*u[29]; A[8] += G[29]*u[29];
#else
#pragma vector unaligned
    for(int k=0; k<Nc; k++){
      G[3*k+0]=0.0;
      G[3*k+0] += jac[3*0+0] * intp_shpg[ip*Ne+ 3*k+0];
      G[3*k+0] += jac[3*0+1] * intp_shpg[ip*Ne+ 3*k+1];
      G[3*k+0] += jac[3*0+2] * intp_shpg[ip*Ne+ 3*k+2];
      A[3*0+0] += G[3*k+0] * u[ndof*k+0];
      A[3*0+1] += G[3*k+0] * u[ndof*k+1];
      A[3*0+2] += G[3*k+0] * u[ndof*k+2];

      G[3*k+1]=0.0;
      G[3*k+1] += jac[3*1+0] * intp_shpg[ip*Ne+ 3*k+0];
      G[3*k+1] += jac[3*1+1] * intp_shpg[ip*Ne+ 3*k+1];
      G[3*k+1] += jac[3*1+2] * intp_shpg[ip*Ne+ 3*k+2];
      A[3*1+0] += G[3*k+1] * u[ndof*k+0];
      A[3*1+1] += G[3*k+1] * u[ndof*k+1];
      A[3*1+2] += G[3*k+1] * u[ndof*k+2];

      G[3*k+2]=0.0;
      G[3*k+2] += jac[3*2+0] * intp_shpg[ip*Ne+ 3*k+0];
      G[3*k+2] += jac[3*2+1] * intp_shpg[ip*Ne+ 3*k+1];
      G[3*k+2] += jac[3*2+2] * intp_shpg[ip*Ne+ 3*k+2];
      A[3*2+0] += G[3*k+2] * u[ndof*k+0];
      A[3*2+1] += G[3*k+2] * u[ndof*k+1];
      A[3*2+2] += G[3*k+2] * u[ndof*k+2];
    };//------------------------------------------------- N*3*6*2 = 36*N FLOP
#endif
#else
    for(int i=0; i< 9 ; i++){ A[i]=0.0;};// H[i]=0.0; B[i]=0.0; };
    //#pragma omp simd
#pragma vector unaligned
    for(int k=0; k<Nc; k++){
#pragma vector unaligned
      for(int i=0; i<3 ; i++){
        G[3*k+i]=0.0;
#pragma vector unaligned
        for(int j=0; j<3 ; j++){
          G[(3* k+i) ] += jac[3* i+j ] * intp_shpg[ip*Ne+ 3* k+j ];
        };
#pragma vector unaligned
        for(int j=0; j<3 ; j++){
          A[(3* i+j) ] += G[(3* k+i) ] * u[ndof* k+j ];
        };
      };
    };//------------------------------------------------- N*3*6*2 = 36*N FLOP
#endif
#endif
#if VERB_MAX>12
    printf( "Small Strains (Elem: %i):", ie );
    for(uint j=0;j<H.size();j++){
      if(j%mesh_d==0){printf("\n");}
      printf("%+9.2e ",H[j]);
    }; printf("\n");
#endif
    dw = jac[9] * wgt[ip];
    if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
      std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj);
      const   INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
#pragma vector unaligned
      for (int i=0; i<Nc; i++){
#if 0
// Slower 44->42
        u[ndof*i+0] = sysu[c[i]*ndof+0];
        u[ndof*i+1] = sysu[c[i]*ndof+1];
        u[ndof*i+2] = sysu[c[i]*ndof+2];
#else
#if 0
// Slower 44->42
#pragma vector unaligned
        for(int j=0; j<3 ; j++) {
          u[ndof*i+j] = sysu[c[i]*ndof+j];
        }
#else
        std::memcpy( &u[ndof*i], &sysu[c[i]*ndof], sizeof(FLOAT_SOLV)*ndof );
#endif
#endif
      };
    }; };
    // [H] Small deformation tensor
    // [H][RT] : matmul3x3x3T
#if 1
#if 0
    __m256d r0,r3,r6,h0,h3,h6,s6,s7,s8;
    __m256d a0,a1,a2,a3,a4,a5,a6,a7,a8;
    r0 = _mm256_loadu_pd(&Rt[0]); r3 = _mm256_loadu_pd(&Rt[3]); r6 = _mm256_loadu_pd(&Rt[6]);
    a0 = _mm256_set1_pd(A[0]) ; a1 = _mm256_set1_pd(A[1]) ; a2 = _mm256_set1_pd(A[2])   ;
    s0 = _mm256_mul_pd(r0,a0) ; s1 = _mm256_mul_pd(r3,a1) ; s2 = _mm256_mul_pd(r6,a2);
    h0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
    _mm256_storeu_pd(&H[0],h0);

    a3 = _mm256_set1_pd(A[3]) ; a4 = _mm256_set1_pd(A[4]) ; a5 = _mm256_set1_pd(A[5]);
    s3 = _mm256_mul_pd(r0,a3) ; s4 = _mm256_mul_pd(r3,a4) ; s5 = _mm256_mul_pd(r6,a5);
    h3 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
    _mm256_storeu_pd(&H[3],h3);

    a6 = _mm256_set1_pd(A[6]) ; a7 = _mm256_set1_pd(A[7]) ; a8 = _mm256_set1_pd(A[8])   ;
    s6 = _mm256_mul_pd(r0,a6) ; s7 = _mm256_mul_pd(r3,a7) ; s8 = _mm256_mul_pd(r6,a8);
    h6 = _mm256_add_pd(s6, _mm256_add_pd(s7,s8));
    _mm256_storeu_pd(&H[6],h6);
#else
#if 0
    H[0]  = A[0]*R[0] + A[1]*R[1] + A[2]*R[2];
    H[1]  = A[0]*R[3] + A[1]*R[4] + A[2]*R[5];
    H[2]  = A[0]*R[6] + A[1]*R[7] + A[2]*R[8];

    H[3]  = A[3]*R[0] + A[4]*R[1] + A[5]*R[2];
    H[4]  = A[3]*R[3] + A[4]*R[4] + A[5]*R[5];
    H[5]  = A[3]*R[6] + A[4]*R[7] + A[5]*R[8];

    H[6]  = A[6]*R[0] + A[7]*R[1] + A[8]*R[2];
    H[7]  = A[6]*R[3] + A[7]*R[4] + A[8]*R[5];
    H[8]  = A[6]*R[6] + A[7]*R[7] + A[8]*R[8];
#else
#endif
#endif
#else
    //#pragma omp simd
#pragma vector unaligned
    for(int i=0; i<3; i++){
#pragma vector unaligned
      for(int k=0; k<3; k++){ H[3* i+k ]=0.0;
#pragma vector unaligned
        for(int j=0; j<3; j++){
          H[(3* i+k) ] += A[(3* i+j)] * R[3* k+j ];
        };};};//---------------------------------------------- 27*2 =      54 FLOP
#endif

#if 1
    __m256d s048;
    __m256d w0 = _mm256_set1_pd(dw);
    s048= _mm256_mul_pd(w0, _mm256_add_pd(_mm256_mul_pd(c0,_mm256_set1_pd(H[0])), _mm256_add_pd(_mm256_mul_pd(c1,_mm256_set1_pd(H[4])), _mm256_mul_pd(c2,_mm256_set1_pd(H[8])))));
          _mm256_storeu_pd(&S[0], s048);
#if 0
    __m256d h0 = _mm256_set_pd(0.,H[5],H[2],H[1]);
    __m256d h1 = _mm256_set_pd(0.,H[7],H[6],H[3]);
          _mm256_storeu_pd(&S[3], _mm256_mul_pd(w0, _mm256_mul_pd(c3, _mm256_add_pd(h0,h1))));
#else
    S[3]=(H[1] + H[3])*C[6]*dw; // S[1]
    S[4]=(H[2] + H[6])*C[8]*dw; // S[2]
    S[5]=(H[5] + H[7])*C[7]*dw; // S[5]
#endif
#else
    //
    S[0]=(C[0]*H[0] + C[3]*H[4] + C[5]*H[8])*dw; S[1]=(H[1] + H[3])*C[6]*dw;                  S[2]=(H[2] + H[6])*C[8]*dw;
    S[4]=(C[3]*H[0] + C[1]*H[4] + C[4]*H[8])*dw; S[5]=(H[5] + H[7])*C[7]*dw;
    S[8]=(C[5]*H[0] + C[4]*H[4] + C[2]*H[8])*dw;
    //S[3]=S[1];                                   S[4]=(C[3]*H[0] + C[1]*H[4] + C[4]*H[8])*dw; S[5]=(H[5] + H[7])*C[7]*dw;
    //S[6]=S[2];                                   S[7]=S[5];                                   S[8]=(C[5]*H[0] + C[4]*H[4] + C[2]*H[8])*dw;//Szz
    //
#endif

#if VERB_MAX>12
    printf( "Stress (Natural Coords):");
    for(uint j=0;j<9;j++){
      if(j%3==0){printf("\n");}
      printf("%+9.2e ",S[j]);
    }; printf("\n");
#endif
    //--------------------------------------------------------- 18+9= 27 FLOP
    // [S][R] : matmul3x3x3, R is transposed
#if 1
#if 1
#if 0
// Slower (44->38)! Need to figure out why, it should be faster because we already have the R's in registers
    S[3]=S[1]; S[6]=S[2]; S[7]=S[5];
    __m256d s0,s3,s6;
    s0 = _mm256_loadu_pd(&S[0]); s3 = _mm256_loadu_pd(&S[3]); s6 = _mm256_loadu_pd(&S[6]);
    a036 = _mm256_add_pd(_mm256_mul_pd(r0,s0), _mm256_add_pd(_mm256_mul_pd(r3,s3),_mm256_mul_pd(r6,s6))); // [?? a2 a1 a0]
    a147 = _mm256_add_pd(_mm256_mul_pd(r1,s0), _mm256_add_pd(_mm256_mul_pd(r4,s3),_mm256_mul_pd(r7,s6))); // [?? a5 a4 a3]
    a258 = _mm256_add_pd(_mm256_mul_pd(r2,s0), _mm256_add_pd(_mm256_mul_pd(r5,s3),_mm256_mul_pd(r8,s6))); // [?? a8 a7 a6]
    _MM256_TRANSPOSE3_PD(a036,a147,a258);
#else
#if 1
    __m256d s0,s1,s2,s4,s5,s8;
    r0  = _mm256_loadu_pd(&R[0]); r3 = _mm256_loadu_pd(&R[3]); r6 = _mm256_loadu_pd(&R[6]);
    s0  = _mm256_set1_pd(S[0])  ; s1 = _mm256_set1_pd(S[3])  ; s2 = _mm256_set1_pd(S[4]);
    s4  = _mm256_set1_pd(S[1])  ; s5 = _mm256_set1_pd(S[5])  ; s8 = _mm256_set1_pd(S[2]);
    a036=_mm256_add_pd(_mm256_mul_pd(r0,s0), _mm256_add_pd(_mm256_mul_pd(r3,s1),_mm256_mul_pd(r6,s2)));
    a147=_mm256_add_pd(_mm256_mul_pd(r0,s1), _mm256_add_pd(_mm256_mul_pd(r3,s4),_mm256_mul_pd(r6,s5)));
    a258=_mm256_add_pd(_mm256_mul_pd(r0,s2), _mm256_add_pd(_mm256_mul_pd(r3,s5),_mm256_mul_pd(r6,s8)));
#else
    __m256d s0,s1,s2,s4,s5,s8;
    r0  = _mm256_loadu_pd(&R[0]); r3 = _mm256_loadu_pd(&R[3]); r6 = _mm256_loadu_pd(&R[6]);
    s0  = _mm256_set1_pd(S[0])  ; s1 = _mm256_set1_pd(S[1])  ; s2 = _mm256_set1_pd(S[2]);
    s4  = _mm256_set1_pd(S[4])  ; s5 = _mm256_set1_pd(S[5])  ; s8 = _mm256_set1_pd(S[8]);
    a036=_mm256_add_pd(_mm256_mul_pd(r0,s0), _mm256_add_pd(_mm256_mul_pd(r3,s1),_mm256_mul_pd(r6,s2)));
    a147=_mm256_add_pd(_mm256_mul_pd(r0,s1), _mm256_add_pd(_mm256_mul_pd(r3,s4),_mm256_mul_pd(r6,s5)));
    a258=_mm256_add_pd(_mm256_mul_pd(r0,s2), _mm256_add_pd(_mm256_mul_pd(r3,s5),_mm256_mul_pd(r6,s8)));
#endif
#endif
#else
    A[0] = S[0]*R[0] + S[1]*R[3] + S[2]*R[6];
    A[3] = S[0]*R[1] + S[1]*R[4] + S[2]*R[7];
    A[6] = S[0]*R[2] + S[1]*R[5] + S[2]*R[8];

    A[1] = S[1]*R[0] + S[4]*R[3] + S[5]*R[6];
    A[4] = S[1]*R[1] + S[4]*R[4] + S[5]*R[7];
    A[7] = S[1]*R[2] + S[4]*R[5] + S[5]*R[8];

    A[2] = S[2]*R[0] + S[5]*R[3] + S[8]*R[6];
    A[5] = S[2]*R[1] + S[5]*R[4] + S[8]*R[7];
    A[8] = S[2]*R[2] + S[5]*R[5] + S[8]*R[8];

    a036 = _mm256_loadu_pd(&A[0]); // [a3 a2 a1 a0]
    a147 = _mm256_loadu_pd(&A[3]); // [a6 a5 a4 a3]
    a258 = _mm256_loadu_pd(&A[6]); // [a9 a8 a7 a6]
    _MM256_TRANSPOSE3_PD(a036,a147,a258);
    __m256d s6,s7,s8;
#endif
#else
    //#pragma omp simd
#pragma vector unaligned
    for(int i=0; i<3; i++){
      //for(int k=0; k<3; k++){ A[3* i+k ]=0.0;
#pragma vector unaligned
      for(int k=0; k<3; k++){ A[3* k+i ]=0.0;
#pragma vector unaligned
        for(int j=0; j<3; j++){
          //A[3* i+k ] += S[3* i+j ] * R[3* j+k ];
          A[(3* k+i) ] += S[(3* i+j) ] * R[3* j+k ];// A is transposed
        };};};//-------------------------------------------------- 27*2 = 54 FLOP
#endif
    //NOTE [A] is not symmetric Cauchy stress.
    //NOTE Cauchy stress is ( A + AT ) /2
#if VERB_MAX>12
    printf( "Rotated Stress (Global Coords):");
    for(uint j=0;j<9;j++){
      if(j%3==0){printf("\n");}
      printf("%+9.2e ",S[j]);
    }; printf("\n");
#endif
#if 1
#if 1
    __m256d g3,g4,g5,g6,g7,g8,g9,g10,g11;
    g0 = _mm256_set1_pd(G[0])  ; g1 = _mm256_set1_pd(G[1])  ; g2 = _mm256_set1_pd(G[2]);
    f0 = _mm256_add_pd(f0, _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));

    g3 = _mm256_set1_pd(G[3])  ; g4 = _mm256_set1_pd(G[4])  ; g5 = _mm256_set1_pd(G[5]);
    f1 = _mm256_add_pd(f1, _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));

    g6 = _mm256_set1_pd(G[6])  ; g7 = _mm256_set1_pd(G[7])  ; g8 = _mm256_set1_pd(G[8]);
    f2 = _mm256_add_pd(f2, _mm256_add_pd(_mm256_mul_pd(g6,a036), _mm256_add_pd(_mm256_mul_pd(g7,a147),_mm256_mul_pd(g8,a258))));

    g9 = _mm256_set1_pd(G[9])  ; g10= _mm256_set1_pd(G[10])  ; g11 = _mm256_set1_pd(G[11]);
    f3 = _mm256_add_pd(f3, _mm256_add_pd(_mm256_mul_pd(g9,a036), _mm256_add_pd(_mm256_mul_pd(g10,a147),_mm256_mul_pd(g11,a258))));

    g0 = _mm256_set1_pd(G[12]) ; g1 = _mm256_set1_pd(G[13]) ; g2 = _mm256_set1_pd(G[14]);
    f4 = _mm256_add_pd(f4, _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));

    g3 = _mm256_set1_pd(G[15]) ; g4 = _mm256_set1_pd(G[16]) ; g5 = _mm256_set1_pd(G[17]);
    f5 = _mm256_add_pd(f5, _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));

    g6 = _mm256_set1_pd(G[18]) ; g7 = _mm256_set1_pd(G[19]) ; g8 = _mm256_set1_pd(G[20]);
    f6 = _mm256_add_pd(f6, _mm256_add_pd(_mm256_mul_pd(g6,a036), _mm256_add_pd(_mm256_mul_pd(g7,a147),_mm256_mul_pd(g8,a258))));

    g9 = _mm256_set1_pd(G[21]) ; g10= _mm256_set1_pd(G[22])  ; g11 = _mm256_set1_pd(G[23]);
    f7 = _mm256_add_pd(f7, _mm256_add_pd(_mm256_mul_pd(g9,a036), _mm256_add_pd(_mm256_mul_pd(g10,a147),_mm256_mul_pd(g11,a258))));

    g0 = _mm256_set1_pd(G[24]) ; g1 = _mm256_set1_pd(G[25]) ; g2 = _mm256_set1_pd(G[26]);
    f8 = _mm256_add_pd(f8, _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));

    g3 = _mm256_set1_pd(G[27]) ; g4 = _mm256_set1_pd(G[28]) ; g5 = _mm256_set1_pd(G[29]);
    f9 = _mm256_add_pd(f9, _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));
#else
// Fully unrolled
    f[0] += G[0]*A[0] + G[1]*A[1] + G[2]*A[2];
    f[1] += G[0]*A[3] + G[1]*A[4] + G[2]*A[5];
    f[2] += G[0]*A[6] + G[1]*A[7] + G[2]*A[8];

    f[3] += G[3]*A[0] + G[4]*A[1] + G[5]*A[2];
    f[4] += G[3]*A[3] + G[4]*A[4] + G[5]*A[5];
    f[5] += G[3]*A[6] + G[4]*A[7] + G[5]*A[8];

    f[6] += G[6]*A[0] + G[7]*A[1] + G[8]*A[2];
    f[7] += G[6]*A[3] + G[7]*A[4] + G[8]*A[5];
    f[8] += G[6]*A[6] + G[7]*A[7] + G[8]*A[8];

    f[ 9] += G[ 9]*A[0] + G[10]*A[1] + G[11]*A[2];
    f[10] += G[ 9]*A[3] + G[10]*A[4] + G[11]*A[5];
    f[11] += G[ 9]*A[6] + G[10]*A[7] + G[11]*A[8];

    f[12] += G[12]*A[0] + G[13]*A[1] + G[14]*A[2];
    f[13] += G[12]*A[3] + G[13]*A[4] + G[14]*A[5];
    f[14] += G[12]*A[6] + G[13]*A[7] + G[14]*A[8];

    f[15] += G[15]*A[0] + G[16]*A[1] + G[17]*A[2];
    f[16] += G[15]*A[3] + G[16]*A[4] + G[17]*A[5];
    f[17] += G[15]*A[6] + G[16]*A[7] + G[17]*A[8];

    f[18] += G[18]*A[0] + G[19]*A[1] + G[20]*A[2];
    f[19] += G[18]*A[3] + G[19]*A[4] + G[20]*A[5];
    f[20] += G[18]*A[6] + G[19]*A[7] + G[20]*A[8];

    f[21] += G[21]*A[0] + G[22]*A[1] + G[23]*A[2];
    f[22] += G[21]*A[3] + G[22]*A[4] + G[23]*A[5];
    f[23] += G[21]*A[6] + G[22]*A[7] + G[23]*A[8];

    f[24] += G[24]*A[0] + G[25]*A[1] + G[26]*A[2];
    f[25] += G[24]*A[3] + G[25]*A[4] + G[26]*A[5];
    f[26] += G[24]*A[6] + G[25]*A[7] + G[26]*A[8];

    f[27] += G[27]*A[0] + G[28]*A[1] + G[29]*A[2];
    f[28] += G[27]*A[3] + G[28]*A[4] + G[29]*A[5];
    f[29] += G[27]*A[6] + G[28]*A[7] + G[29]*A[8];
#endif
#else
    //#pragma omp simd
#pragma vector unaligned
    for(int i=0; i<Nc; i++){
#pragma vector unaligned
      for(int k=0; k<3; k++){
#pragma vector unaligned
        for(int j=0; j<3; j++){
          f[(3* i+k) ] += G[(3* i+j) ] * A[(3* k+j) ];
        };};};//---------------------------------------------- N*3*6 = 18*N FLOP
#endif
#if VERB_MAX>12
    printf( "ff:");
    for(uint j=0;j<Ne;j++){
      if(j%mesh_d==0){printf("\n");}
      printf("%+9.2e ",f[j]);
    }; printf("\n");
#endif
    };//end intp loop

    _mm256_storeu_pd(&f[0],f0);
    _mm256_storeu_pd(&f[3],f1);
    _mm256_storeu_pd(&f[6],f2);
    _mm256_storeu_pd(&f[9],f3);
    _mm256_storeu_pd(&f[12],f4);
    _mm256_storeu_pd(&f[15],f5);
    _mm256_storeu_pd(&f[18],f6);
    _mm256_storeu_pd(&f[21],f7);
    _mm256_storeu_pd(&f[24],f8);
    _mm256_storeu_pd(&f[27],f9);
#if 0
#if 1
    //_mm256_storeu_pd(&sys_f[3*conn[0]],f0);
    //_mm256_storeu_pd(&sys_f[3*conn[1]],f1);
    //_mm256_storeu_pd(&sys_f[3*conn[2]],f2);
    //_mm256_storeu_pd(&sys_f[3*conn[3]],f3);
    //_mm256_storeu_pd(&sys_f[3*conn[4]],f4);
    //_mm256_storeu_pd(&sys_f[3*conn[5]],f5);
    //_mm256_storeu_pd(&sys_f[3*conn[6]],f6);
    //_mm256_storeu_pd(&sys_f[3*conn[7]],f7);
    //_mm256_storeu_pd(&sys_f[3*conn[8]],f8);
    //_mm256_storeu_pd(&sys_f[3*conn[9]],f9);

    _mm_storeu_pd(&sys_f[3*conn[0]],_mm256_castpd256_pd128(f0));
    _mm_store_sd (&sys_f[3*conn[0]+2],_mm256_extractf128_pd(f0,0x1));
    _mm_storeu_pd(&sys_f[3*conn[1]],_mm256_castpd256_pd128(f1));
    _mm_store_sd (&sys_f[3*conn[1]+2],_mm256_extractf128_pd(f1,0x1));
    _mm_storeu_pd(&sys_f[3*conn[2]],_mm256_castpd256_pd128(f2));
    _mm_store_sd (&sys_f[3*conn[2]+2],_mm256_extractf128_pd(f2,0x1));
    _mm_storeu_pd(&sys_f[3*conn[3]],_mm256_castpd256_pd128(f3));
    _mm_store_sd (&sys_f[3*conn[3]+2],_mm256_extractf128_pd(f3,0x1));
    _mm_storeu_pd(&sys_f[3*conn[4]],_mm256_castpd256_pd128(f4));
    _mm_store_sd (&sys_f[3*conn[4]+2],_mm256_extractf128_pd(f4,0x1));
    _mm_storeu_pd(&sys_f[3*conn[5]],_mm256_castpd256_pd128(f5));
    _mm_store_sd (&sys_f[3*conn[5]+2],_mm256_extractf128_pd(f5,0x1));
    _mm_storeu_pd(&sys_f[3*conn[6]],_mm256_castpd256_pd128(f6));
    _mm_store_sd (&sys_f[3*conn[6]+2],_mm256_extractf128_pd(f6,0x1));
    _mm_storeu_pd(&sys_f[3*conn[7]],_mm256_castpd256_pd128(f7));
    _mm_store_sd (&sys_f[3*conn[7]+2],_mm256_extractf128_pd(f7,0x1));
    _mm_storeu_pd(&sys_f[3*conn[8]],_mm256_castpd256_pd128(f8));
    _mm_store_sd (&sys_f[3*conn[8]+2],_mm256_extractf128_pd(f8,0x1));
    _mm_storeu_pd(&sys_f[3*conn[9]],_mm256_castpd256_pd128(f9));
    _mm_store_sd (&sys_f[3*conn[9]+2],_mm256_extractf128_pd(f9,0x1));
#else
    sys_f[3*conn[0]+0] = f[ 0]; sys_f[3*conn[0]+1] = f[ 1]; sys_f[3*conn[0]+2] = f[ 2];
    sys_f[3*conn[1]+0] = f[ 3]; sys_f[3*conn[1]+1] = f[ 4]; sys_f[3*conn[1]+2] = f[ 5];
    sys_f[3*conn[2]+0] = f[ 6]; sys_f[3*conn[2]+1] = f[ 7]; sys_f[3*conn[2]+2] = f[ 8];
    sys_f[3*conn[3]+0] = f[ 9]; sys_f[3*conn[3]+1] = f[10]; sys_f[3*conn[3]+2] = f[11];
    sys_f[3*conn[4]+0] = f[12]; sys_f[3*conn[4]+1] = f[13]; sys_f[3*conn[4]+2] = f[14];
    sys_f[3*conn[5]+0] = f[15]; sys_f[3*conn[5]+1] = f[16]; sys_f[3*conn[5]+2] = f[17];
    sys_f[3*conn[6]+0] = f[18]; sys_f[3*conn[6]+1] = f[19]; sys_f[3*conn[6]+2] = f[20];
    sys_f[3*conn[7]+0] = f[21]; sys_f[3*conn[7]+1] = f[22]; sys_f[3*conn[7]+2] = f[23];
    sys_f[3*conn[8]+0] = f[24]; sys_f[3*conn[8]+1] = f[25]; sys_f[3*conn[8]+2] = f[26];
    sys_f[3*conn[9]+0] = f[27]; sys_f[3*conn[9]+1] = f[28]; sys_f[3*conn[9]+2] = f[29];
#endif
#else
#if 0
//Slower! 44->41
#pragma vector unaligned
    for (int i=0; i<Nc; i++){
      std::memcpy(& sys_f[Econn[Nc*ie+i]*Nf],& f[Nf*i], sizeof(FLOAT_SOLV)*Nf ); };
#else

#pragma vector unaligned
    for (int i=0; i<Nc; i++){
#pragma vector unaligned
      for(int j=0; j<3; j++){
#if 0
//Slower! 44->40
        sys_f[3*Econn[Nc*ie+i]+j] = f[(3*i+j)];
#else
        sys_f[3*conn[i]+j] = f[(3*i+j)];
#endif
      }; };//--------------------------------------------------- N*3 =  3*N FLOP
#endif
#endif
    //if(fetch_next){
    //  std::memcpy( &conn, &Econn[Nc*(ie+1)], sizeof(  INT_MESH)*Nc); };
};//end elem loop
return 0;
};
int ElastOrtho3D::BlocLinear( Elem* E, RESTRICT Phys::vals &sys_f, const RESTRICT Phys::vals &sys_u ){
  //FIXME Cleanup local variables.
  const uint ndof   = 3;//this->node_d
  //const int mesh_d = 3;//E->elem_d;
  const uint  Nj =10;//,d2=9;//mesh_d*mesh_d;
  //
  const INT_MESH elem_n = E->elem_n;
  const uint     Nc = E->elem_conn_n;// Number of Nodes/Element
  const uint     Ne = ndof*Nc;
  const uint intp_n = uint(E->gaus_n);
  uint           Nv = E->simd_n;// Vector block size
  //
  INT_MESH e0=0, ee=elem_n;
  if(E->do_halo==true){ e0=0; ee=E->halo_elem_n;
  }else{ e0=E->halo_elem_n; ee=elem_n; };
  //
#if VERB_MAX>11
  printf("Dim: %i, Elems:%i, IntPts:%i, Nodes/elem:%i\n",
      (int)mesh_d,(int)elem_n,(int)intp_n,(int)Nc);
#endif
  //INT_MESH   conn[Nc*Nv];
  FLOAT_PHYS G[Ne*Nv], u[Ne*Nv],f[Ne*Nv];
  //FLOAT_PHYS jac[Nj*Nv], det;
  FLOAT_PHYS A[9*Nv], B[9*Nv], H[9*Nv], S[9*Nv];
  //
  FLOAT_PHYS intp_shpg[intp_n*Ne];
  std::copy( &E->intp_shpg[0],// local copy
      &E->intp_shpg[intp_n*Ne], intp_shpg );
  FLOAT_PHYS wgt[intp_n];
  std::copy( &E->gaus_weig[0],
      &E->gaus_weig[intp_n], wgt );
  FLOAT_PHYS C[this->mtrl_matc.size()];
  std::copy( &this->mtrl_matc[0],
      &this->mtrl_matc[this->mtrl_matc.size()], C );
  const FLOAT_PHYS R[9] = {
    mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
    mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
    mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8]};
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",mtrl_matc[j]);
  }; printf("\n");
#endif
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu0 = &sys_u[0];
  for(INT_MESH ie=e0;ie<ee;ie+=Nv){
    if( (ie+Nv)>=ee ){ Nv=ee-ie; };
    const   INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    const FLOAT_MESH* RESTRICT jac  = &Ejacs[Nj*ie];
    //
    //std::memcpy( &conn, &Econn[Nc*ie], sizeof(  INT_MESH)*Nc*Nv);
    //std::memcpy( &jac , &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj*Nv);
    //std::copy( &E->elem_conn[Nc*ie],
    //           &E->elem_conn[Nc*ie+Nc*Nv], conn );
    //std::copy( &E->elip_jacs[Nj*ie],
    //           &E->elip_jacs[Nj*ie+Nj*Nv], jac );// det=jac[9];
    for (uint i=0; i<(Nc*Nv); i++){
      std::memcpy( &    u[ndof*i],
          //&sys_u[conn[i]*ndof], sizeof(FLOAT_SOLV)*ndof ); };
        &sysu0[conn[i]*ndof], sizeof(FLOAT_SOLV)*ndof ); };
  for(uint i=0;i<(Ne*Nv);i++){ f[i]=0.0; };
  for(uint ip=0; ip<intp_n; ip++){
    //G = MatMul3x3xN( jac,shg );
    //A = MatMul3xNx3T( G,u );
    for(uint i=0; i< 9*Nv ; i++){ H[i]=0.0; A[i]=0.0; B[i]=0.0; };
    for(uint i=0; i<(Ne*Nv) ; i++){ G[i]=0.0; };
#pragma omp simd collapse(1)
    for(uint l=0;l<Nv;l++){
      for(uint k=0; k<Nc; k++){
        for(uint i=0; i<3 ; i++){// G[3* k+i ]=0.0;
          for(uint j=0; j<3 ; j++){
            //FIXME can this vectorize?
            G[(3* k+i)*Nv+l ] += jac[3* j+i +l*Nj ] * intp_shpg[ip*Ne+ 3* k+j ];
          };
        };
      };
    };
#pragma omp simd collapse(1)
    for(uint l=0;l<Nv;l++){
      for(uint k=0; k<Nc; k++){
        for(uint i=0; i<3 ; i++){
          for(uint j=0; j<3 ; j++){
            A[(3* i+j)*Nv+l ] += G[(3* k+i)*Nv+l ] * u[ndof* k+j +l*Ne ];
          };
        };
      };
    };//------------------------------------------------- N*3*6*2 = 36*N FLOP
#if VERB_MAX>10
    printf( "Small Strains (Elem: %i):", ie );
    for(uint j=0;j<HH.size();j++){
      if(j%mesh_d==0){printf("\n");}
      printf("%+9.2e ",H[j]);
    }; printf("\n");
#endif
    // [H] Small deformation tensor
    // [H][RT] : matmul3x3x3T
#pragma omp simd collapse(1)
    for(uint l=0;l<Nv;l++){
      for(uint i=0; i<3; i++){
        for(uint k=0; k<3; k++){// H[3* i+k ]=0.0;
          for(uint j=0; j<3; j++){
            H[(3* i+k)*Nv+l ] += A[(3* i+j)*Nv +l] * R[3* k+j ]; };
        };};};//---------------------------------------------- 27*2 =      54 FLOP
#pragma omp simd
    for(uint l=0;l<Nv;l++){
      //det=jac[9 +Nj*l]; FLOAT_PHYS w = det * wgt[ip];
      FLOAT_PHYS w = jac[9 +Nj*l] * wgt[ip];
      //
      S[0*Nv+l]=(C[0]* H[0*Nv+l] + C[3]* H[4*Nv+l] + C[5]* H[8*Nv+l])*w;//Sxx
      S[4*Nv+l]=(C[3]* H[0*Nv+l] + C[1]* H[4*Nv+l] + C[4]* H[8*Nv+l])*w;//Syy
      S[8*Nv+l]=(C[5]* H[0*Nv+l] + C[4]* H[4*Nv+l] + C[2]* H[8*Nv+l])*w;//Szz
      //
      S[1*Nv+l]=( H[1*Nv+l] + H[3*Nv+l] )*C[6]*w;// S[3]= S[1];//Sxy Syx
      S[5*Nv+l]=( H[5*Nv+l] + H[7*Nv+l] )*C[7]*w;// S[7]= S[5];//Syz Szy
      S[2*Nv+l]=( H[2*Nv+l] + H[6*Nv+l] )*C[8]*w;// S[6]= S[2];//Sxz Szx
      S[3*Nv+l]=S[1*Nv+l]; S[7*Nv+l]=S[5*Nv+l]; S[6*Nv+l]=S[2*Nv+l];
    };
#if VERB_MAX>10
    printf( "Stress (Natural Coords):");
    for(uint j=0;j<9;j++){
      if(j%3==0){printf("\n");}
      printf("%+9.2e ",S[j]);
    }; printf("\n");
#endif
    //--------------------------------------------------------- 18+9= 27 FLOP
    // [S][R] : matmul3x3x3, R is transposed
    //for(int i=0; i<9; i++){ B[i]=0.0; };
#pragma omp simd collapse(1)
    for(uint l=0;l<Nv;l++){
      for(uint i=0; i<3; i++){
        //for(int k=0; k<3; k++){ B[3* i+k ]=0.0;
        for(uint k=0; k<3; k++){// B[3* k+i ]=0.0;
          for(uint j=0; j<3; j++){
            //B[3* i+k ] += S[3* i+j ] * R[3* j+k ];
            B[(3* k+i)*Nv+l ] += S[(3* i+j)*Nv+l ] * R[3* j+k ]; };// B is transposed
        };};};//-------------------------------------------------- 27*2 = 54 FLOP
      //NOTE [B] is not symmetric Cauchy stress.
      //NOTE Cauchy stress is ( B + BT ) /2
#if VERB_MAX>10
      printf( "Rotated Stress (Global Coords):");
      for(uint j=0;j<9;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e ",S[j]);
      }; printf("\n");
#endif
#pragma omp simd collapse(1)
      for(uint l=0;l<Nv;l++){
        for(uint i=0; i<Nc; i++){
          for(uint k=0; k<3; k++){
            for(uint j=0; j<3; j++){
              f[(3* i+k)*Nv+l ] += G[(3* i+j)*Nv+l ] * B[(3* k+j)*Nv+l ]; };
          };};};//---------------------------------------------- N*3*6 = 18*N FLOP
      // This is way slower:
      //for(uint i=0; i<Nc; i++){
      //  for(uint k=0; k<3 ; k++){
      //    for(uint j=0; j<3 ; j++){
      //    for(uint l=0; l<3 ; l++){
      //      f[3* i+l ] += G[3* i+j ] * S[3* j+k ] * R[3* k+l ];
      //};};};};
#if VERB_MAX>10
      printf( "ff:");
      for(uint j=0;j<Ne;j++){
        if(j%mesh_d==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      }; printf("\n");
#endif
    };//end intp loop
    for (uint i=0; i<Nc; i++){
      for(uint j=0; j<3; j++){
#pragma omp simd
        for(uint l=0;l<Nv;l++){//Cv
          sys_f[3*conn[i+Nc*l]+j] += f[(3*i+j)*Nv+l];
        };
      }; };//--------------------------------------------------- N*3 =  3*N FLOP
  };//end elem loop
  return 0;
};
int ElastOrtho3D::ElemJacobi(Elem* E, RESTRICT Phys::vals &sys_d ){
  //FIXME Doesn't do rotation yet
  const uint ndof   = 3;//this->node_d
  //const int mesh_d = E->elem_d;
  const uint elem_n = E->elem_n;
  const uint  Nc = E->elem_conn_n;
  const uint  Nj = 10,d2=9;
  const uint  Ne = ndof*Nc;
  const uint intp_n = uint(E->gaus_n);
  //
  FLOAT_PHYS det;
  FLOAT_PHYS elem_diag[Ne];
  FLOAT_PHYS B[Ne*6];// 6 rows, Ne cols
  FLOAT_PHYS G[Ne],jac[Nj];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; };
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[3],mtrl_matc[5],0.0,0.0,0.0,
    mtrl_matc[3],mtrl_matc[1],mtrl_matc[4],0.0,0.0,0.0,
    mtrl_matc[5],mtrl_matc[4],mtrl_matc[2],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[6],0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[7],0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[8]};
  for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;
    std::copy( &E->elip_jacs[ij],
        &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint i=0;i<Ne;i++){ elem_diag[i]=0.0; };
    for(uint ip=0;ip<intp_n;ip++){
      //G   = MatMul3x3xN(jac,shg);
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; };
      for(uint k=0;k<Nc;k++){
        for(uint i=0;i<3;i++){
          for(uint j=0;j<3;j++){
            G[3* i+k] += jac[3* j+i ] * E->intp_shpg[ig+3* k+j ]; }; }; };
#if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e",jac[j]);
      }; printf(" det:%+9.2e\n",det);
#endif
      // xx yy zz
      for(uint j=0; j<Nc; j++){
        B[Ne*0 + 0+j*ndof] = G[Nc*0+j];
        B[Ne*1 + 1+j*ndof] = G[Nc*1+j];
        B[Ne*2 + 2+j*ndof] = G[Nc*2+j];
        // xy yx
        B[Ne*3 + 0+j*ndof] = G[Nc*1+j];
        B[Ne*3 + 1+j*ndof] = G[Nc*0+j];
        // yz zy
        B[Ne*4 + 1+j*ndof] = G[Nc*2+j];
        B[Ne*4 + 2+j*ndof] = G[Nc*1+j];
        // xz zx
        B[Ne*5 + 0+j*ndof] = G[Nc*2+j];
        B[Ne*5 + 2+j*ndof] = G[Nc*0+j];
      };
#if VERB_MAX>10
      printf( "[B]:");
      for(uint j=0;j<B.size();j++){
        if(j%Ne==0){printf("\n");}
        printf("%+9.2e ",B[j]);
      }; printf("\n");
#endif
      FLOAT_PHYS w = det * E->gaus_weig[ip];
      for(uint i=0; i<Ne; i++){
        for(uint k=0; k<6 ; k++){
          //for(uint l=0; l<6 ; l++){
          //K[Ne*i+l]+= B[Ne*j + ij] * D[6*j + k] * B[Ne*k + l];
          //uint l=i;
          //elem_d[ie*Ne+i]+= B[Ne*j + i] * D[6*j + k] * B[Ne*k + i];
          elem_diag[i]+=(B[Ne*0 + i] * D[6*0 + k] * B[Ne*k + i])*w;
          elem_diag[i]+=(B[Ne*1 + i] * D[6*1 + k] * B[Ne*k + i])*w;
          elem_diag[i]+=(B[Ne*2 + i] * D[6*2 + k] * B[Ne*k + i])*w;
          elem_diag[i]+=(B[Ne*3 + i] * D[6*3 + k] * B[Ne*k + i])*w;
          elem_diag[i]+=(B[Ne*4 + i] * D[6*4 + k] * B[Ne*k + i])*w;
          elem_diag[i]+=(B[Ne*5 + i] * D[6*5 + k] * B[Ne*k + i])*w;
        };};//};//};
  };//end intp loop
  for (uint i=0; i<Nc; i++){
    //int c=E->elem_conn[Nc*ie+i]*3;
    for(uint j=0; j<3; j++){
      sys_d[E->elem_conn[Nc*ie+i]*3+j] += elem_diag[3*i+j];
    }; };
  //elem_diag=0.0;
};
return 0;
};

int ElastOrtho3D::ElemRowSumAbs(Elem* E, RESTRICT Phys::vals &sys_d ){
  //FIXME Doesn't do rotation yet
  const uint ndof   = 3;//this->node_d
  //const int mesh_d = E->elem_d;
  const uint elem_n = E->elem_n;
  const uint  Nc = E->elem_conn_n;
  const uint  Nj = 10,d2=9;
  const uint  Ne = uint(ndof*Nc);
  const uint intp_n = E->gaus_n;
  //
  FLOAT_PHYS det;
  FLOAT_PHYS elem_sum[Ne];
  FLOAT_PHYS K[Ne*Ne];
  //RESTRICT Phys::vals B(Ne*6);
  FLOAT_PHYS B[Ne*6];//6 rows, Ne cols
  FLOAT_PHYS G[Ne],jac[Nj];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; };
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[1],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[0],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[1],mtrl_matc[0],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[2],0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[2],0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[2]};
  for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;//FIXME only good for tets
    std::copy( &E->elip_jacs[ij],
        &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint i=0;i<Ne;i++){ elem_sum[i]=0.0; };
    for(uint i=0;i<(Ne*Ne);i++){ K[i]=0.0; };
    for(uint ip=0;ip<intp_n;ip++){
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; };
      for(uint k=0;k<Nc;k++){
        for(uint i=0;i<3;i++){
          for(uint j=0;j<3;j++){
            G[3* i+k] += jac[3* j+i] * E->intp_shpg[ig+3* k+j]; }; }; };
#if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e",jac[j]);
      }; printf(" det:%+9.2e\n",det);
#endif
      // xx yy zz
      for(uint j=0; j<Nc; j++){
        B[Ne*0 + 0+j*ndof] = G[Nc*0+j];
        B[Ne*1 + 1+j*ndof] = G[Nc*1+j];
        B[Ne*2 + 2+j*ndof] = G[Nc*2+j];
        // xy yx
        B[Ne*3 + 0+j*ndof] = G[Nc*1+j];
        B[Ne*3 + 1+j*ndof] = G[Nc*0+j];
        // yz zy
        B[Ne*4 + 1+j*ndof] = G[Nc*2+j];
        B[Ne*4 + 2+j*ndof] = G[Nc*1+j];
        // xz zx
        B[Ne*5 + 0+j*ndof] = G[Nc*2+j];
        B[Ne*5 + 2+j*ndof] = G[Nc*0+j];
      };
#if VERB_MAX>10
      printf( "[B]:");
      for(uint j=0;j<B.size();j++){
        if(j%Ne==0){printf("\n");}
        printf("%+9.2e ",B[j]);
      }; printf("\n");
#endif
      FLOAT_PHYS w = det * E->gaus_weig[ip];
      for(uint i=0; i<Ne; i++){
        for(uint l=0; l<Ne; l++){
          for(uint j=0; j<6 ; j++){
            for(uint k=0; k<6 ; k++){
              K[Ne*i+l]+= B[Ne*j + i] * D[6*j + k] * B[Ne*k + l]*w;
              //elem_sum[i]+=std::abs(B[Ne*j + i] * D[6*j + k] * B[Ne*k + l])*w; };
          };};};};
  };//end intp loop
  for (uint i=0; i<Ne; i++){
    for(uint j=0; j<Ne; j++){
      //elem_sum[i] += K[Ne*i+j]*K[Ne*i+j];
      elem_sum[i] += std::abs(K[Ne*i+j]);
    };};
  for (uint i=0; i<Nc; i++){
    for(uint j=0; j<3; j++){
      sys_d[E->elem_conn[Nc*ie+i]*3+j] += elem_sum[3*i+j];
    };};
  //K=0.0; elem_sum=0.0;
};
return 0;
};
int ElastOrtho3D::ElemStrain( Elem* E,
    RESTRICT Phys::vals &sys_f ){
  //FIXME Clean up local variables.
  const uint ndof= 3;//this->node_d
  const uint  Nj =10;//,d2=9;//mesh_d*mesh_d;
  const INT_MESH elem_n = E->elem_n;
  const uint intp_n = uint(E->gaus_n);
  const uint     Nc = E->elem_conn_n;// Number of Nodes/Element
  const uint     Ne = ndof*Nc;
  //FLOAT_PHYS det;
  INT_MESH   conn[Nc];
  FLOAT_MESH jac[Nj];
  FLOAT_PHYS dw, G[Ne], f[Ne];
  FLOAT_PHYS H[9], S[9], A[9], B[9];
  //
  for(uint i=0; i< 9 ; i++){ A[i]=0.0; };
  for(uint i=0; i< 9 ; i++){ H[i]=0.0; };
  H[0]=1.0; H[4]=1.0; H[8]=1.0;// unit pressure
  //
  FLOAT_PHYS intp_shpg[intp_n*Ne];
  std::copy( &E->intp_shpg[0],
      &E->intp_shpg[intp_n*Ne], intp_shpg );
  FLOAT_PHYS wgt[intp_n];
  std::copy( &E->gaus_weig[0],
      &E->gaus_weig[intp_n], wgt );
  FLOAT_PHYS C[this->mtrl_matc.size()];
  std::copy( &this->mtrl_matc[0],
      &this->mtrl_matc[this->mtrl_matc.size()], C );
  const FLOAT_PHYS R[9] = {
    mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
    mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
    mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8]};
  for(uint i=0; i<3; i++){
    for(uint k=0; k<3; k++){
      for(uint j=0; j<3; j++){
        H[(3* i+k) ] += A[(3* i+j)] * R[3* k+j ];
      };};};
  const auto Econn = &E->elem_conn[0];
  const auto Ejacs = &E->elip_jacs[0];
  //
  for(INT_MESH ie=0;ie<elem_n;ie++){
    std::memcpy( &conn, &Econn[Nc*ie], sizeof(  INT_MESH)*Nc);
    std::memcpy( &jac , &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
    //
    for(uint i=0;i<(Ne);i++){ f[i]=0.0; };
    for(uint ip=0; ip<intp_n; ip++){
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      //for(uint i=0; i<(Ne) ; i++){ G[i]=0.0; };
      for(uint k=0; k<Nc; k++){
        for(uint i=0; i<3 ; i++){ G[3* k+i ]=0.0;
          for(uint j=0; j<3 ; j++){
            G[(3* k+i) ] += jac[3* j+i ] * intp_shpg[ip*Ne+ 3* k+j ];
          };
        };
      };//------------------------------------------------- N*3*6*2 = 36*N FLOP
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(uint j=0;j<HH.size();j++){
        if(j%mesh_d==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      }; printf("\n");
#endif
      //det=jac[9 +Nj*l]; FLOAT_PHYS w = det * wgt[ip];
      dw = jac[9] * wgt[ip];
      //
      S[0]=(C[0]* H[0] + C[1]* H[4] + C[1]* H[8])*dw;//Sxx
      S[4]=(C[1]* H[0] + C[0]* H[4] + C[1]* H[8])*dw;//Syy
      S[8]=(C[1]* H[0] + C[1]* H[4] + C[0]* H[8])*dw;//Szz
      //
      S[1]=( H[1] + H[3] )*C[2]*dw;// S[3]= S[1];//Sxy Syx
      S[5]=( H[5] + H[7] )*C[2]*dw;// S[7]= S[5];//Syz Szy
      S[2]=( H[2] + H[6] )*C[2]*dw;// S[6]= S[2];//Sxz Szx
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
      //------------------------------------------------------- 18+9 = 27 FLOP
      for(uint i=0; i<3; i++){
        //for(int k=0; k<3; k++){ A[3* i+k ]=0.0;
        for(uint k=0; k<3; k++){ B[3* k+i ]=0.0;
          for(uint j=0; j<3; j++){
            //A[3* i+k ] += S[3* i+j ] * R[3* j+k ];
            B[(3* k+i) ] += S[(3* i+j) ] * R[3* j+k ];// B is transposed
          };};};
      for(uint i=0; i<Nc; i++){
        for(uint k=0; k<3; k++){
          for(uint j=0; j<3; j++){
            f[(3* i+k) ] += G[(3* i+j) ] * B[(3* k+j) ];
          };};};//---------------------------------------------- N*3*6 = 18*N FLOP
#if VERB_MAX>10
      printf( "f:");
      for(uint j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      }; printf("\n");
#endif
      };//end intp loop
      for (uint i=0; i<Nc; i++){
        for(uint j=0; j<3; j++){
          //sys_f[3*conn[i]+j] +=f[(3*i+j)];
          sys_f[3*conn[i]+j] += std::abs( f[(3*i+j)] );
        }; };//--------------------------------------------------- N*3 =  3*N FLOP
    };//end elem loop
    return 0;
  };
