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
int ElastIso3D::Setup( Elem* E ){
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10) ;
  const uint intp_n = uint(E->gaus_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (54) + 27 );
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(3*conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n // Main mem ints
    +sizeof(FLOAT_PHYS)*(3*intp_n*conn_n +3+1 ) );// Stack (assumes read once)
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) +2);
  return 0;
};
int ElastIso3D::ElemLinear( Elem* E,
  FLOAT_SOLV* sys_f, const FLOAT_SOLV* sys_u ){
  //FIXME Clean up local variables.
  //const int De = 3;// Element Dimension
  const int Nd = 3;// Node (mesh) Dimension
  const int Nf = 3;// this->node_d DOF/node
  const int Nj = Nd*Nf+1;//FIXME wrong?
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nf*Nc;
  const INT_MESH elem_n =E->elem_n;
  const int intp_n = int(E->gaus_n);
  //
  INT_MESH e0=0, ee=elem_n;
  if(E->do_halo==true){ e0=0; ee=E->halo_elem_n;
  }else{ e0=E->halo_elem_n; ee=elem_n;};
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
  FLOAT_MESH __attribute__((aligned(64))) jac[Nj];
  FLOAT_PHYS __attribute__((aligned(64))) dw, G[Ne+1], u[Ne+1], f[Ne+1];
  FLOAT_PHYS __attribute__((aligned(64))) H[Nd*Nf+1], S[Nd*Nf];
  //
  FLOAT_PHYS __attribute__((aligned(64))) intp_shpg[intp_n*Ne];
  std::copy( &E->intp_shpg[0],
             &E->intp_shpg[intp_n*Ne], intp_shpg );
  FLOAT_PHYS __attribute__((aligned(64))) wgt[intp_n];
  std::copy( &E->gaus_weig[0],
             &E->gaus_weig[intp_n], wgt );
  FLOAT_PHYS __attribute__((aligned(64))) C[this->mtrl_matc.size()];
  std::copy( &this->mtrl_matc[0],
             &this->mtrl_matc[this->mtrl_matc.size()], C );
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
        FLOAT_SOLV* RESTRICT sysf  = &sys_f[0];
  if(e0<ee){
#ifdef FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i],& sysu[Econn[Nc*e0+i]*Nf],
        sizeof(FLOAT_SOLV)*Nf ); };
  };
  for(INT_MESH ie=e0;ie<ee;ie++){
#ifndef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    for (int i=0; i<Nc; i++){
      std::memcpy( & f[Nf*i],& sysf[conn[i]*3], sizeof(FLOAT_SOLV)*Nf ); }
      __m256d j0,j1,j2;
      j0 = _mm256_loadu_pd(&jac[0]);  // j0 = [j3 j2 j1 j0]
      j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
      j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]
      for(int ip=0; ip<intp_n; ip++){
        //G = MatMul3x3xN( jac,shg );
        //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
#if 1
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
#if 1
    _mm256_storeu_pd(&H[0],a036);
    _mm256_storeu_pd(&H[3],a147);
    _mm256_storeu_pd(&H[6],a258);
#else
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
      for(int i=0; i< 9 ; i++){ H[i]=0.0; };
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Nf ; k++){ G[Nf* i+k ]=0.0;
          for(int j=0; j<Nd ; j++){
            G[(Nf* i+k) ] += intp_shpg[ip*Ne+ Nd* i+j ] * jac[Nd* j+k ];
          };
          for(int j=0; j<Nf ; j++){
            H[(Nf* k+j) ] += G[(Nf* i+k) ] * u[Nf* i+j ];
          };
        };// 36*N FMA FLOP
      };//------------------------------------------------ N*3*6*2 = 36*N FLOP
#endif
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<H.size();j++){
        if(j%mesh_d==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      }; printf("\n");
#endif
      dw = jac[9] * wgt[ip];
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
        const INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
        for (int i=0; i<Nc; i++){
          std::memcpy(& u[Nf*i],& sysu[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); };
#ifdef FETCH_JAC
        std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj);
#endif
      }; };
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
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Nf; k++){
          for(int j=0; j<Nf; j++){
            f[(Nf* i+k) ] += G[(Nf* i+j) ] * S[(Nf* j+k) ];// 18*N FMA FLOP
      };};};//---------------------------------------------- N*3*6 = 18*N FLOP
#if VERB_MAX>10
      printf( "f:");
      for(int j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      }; printf("\n");
#endif
    };//end intp loop
    for (uint i=0; i<uint(Nc); i++){
      std::memcpy(& sysf[conn[i]*3],& f[Nf*i], sizeof(FLOAT_SOLV)*Nf );
#if 0
      if( n >=my_node_start ){
        for(uint j=0;j<3;j++){
          this->part_sum1+= f[Nf* i+j ] * sysu[Nf* n+j ];
          //FIXME u already contains next elem sys_u
          //this->part_sum1+= f[Nf* i+j ] * u[Nf* i+j];
        };
      };
#endif
    };
  };//end elem loop
  return 0;
  };