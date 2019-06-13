#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>

// Vectorize f calculation
#undef VEC_F
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
  const int Nj = Nd*Nd+1;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nf*Nc;
  const INT_MESH elem_n =E->elem_n;
  const int intp_n = int(E->gaus_n);
  const INT_ORDER elem_p =E->elem_p;
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
#if 0
  __m256d c0,c1,c2,c3;
  c0 = _mm256_set_pd(0.,C[5],C[3],C[0]); c1 = _mm256_set_pd(0.,C[4],C[1],C[3]); c2 = _mm256_set_pd(0.,C[2],C[4],C[5]); c3 = _mm256_set_pd(0.,C[7],C[8],C[6]);
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
        FLOAT_SOLV* RESTRICT sysf  = &sys_f[0];
  if(e0<ee){
#ifdef FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT c = &Econn[Nc*e0];
#pragma vector unaligned
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i],&sysu[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf ); }
  }
  for(INT_MESH ie=e0;ie<ee;ie++){//================================== Elem loop
#ifndef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
#ifdef VEC_F
// This is slower
    __m256d f0,f1,f2,f3,f4,f5,f6,f7,f8,f9;
    f0 = _mm256_loadu_pd(&sysf[3*conn[0]]); f1 = _mm256_loadu_pd(&sysf[3*conn[1]]); f2 = _mm256_loadu_pd(&sysf[3*conn[2]]);
    f3 = _mm256_loadu_pd(&sysf[3*conn[3]]); f4 = _mm256_loadu_pd(&sysf[3*conn[4]]); f5 = _mm256_loadu_pd(&sysf[3*conn[5]]);
    f6 = _mm256_loadu_pd(&sysf[3*conn[6]]); f7 = _mm256_loadu_pd(&sysf[3*conn[7]]); f8 = _mm256_loadu_pd(&sysf[3*conn[8]]);
    f9 = _mm256_loadu_pd(&sysf[3*conn[9]]);
#else
#pragma vector unaligned
    for (int i=0; i<Nc; i++){
      std::memcpy( & f[Nf*i],& sysf[conn[i]*3], sizeof(FLOAT_SOLV)*Nf ); }
#endif
    __m256d j0,j1,j2;
    j0 = _mm256_loadu_pd(&jac[0]);  // j0 = [j3 j2 j1 j0]
    j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
    j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]
    for(int ip=0; ip<intp_n; ip++){//============================== Int pt loop
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
#if 1
      {// scope vector registers
      __m256d a036, a147, a258;
      __m256d u0,u1,u2,u3,u4,u5,g0,g1;
      __m256d is0,is1,is2,is3,is4,is5;
      double * RESTRICT isp = &intp_shpg[ip*Ne];
      // (6*0 +4)*3 = (4)*3 = 12
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
      if(elem_p>1){// (6*1 +4)*3 = (10)*3 = 30

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
      }
      if(elem_p>2){// (6*2 +4)*3 = (16)*3 = 48
        is0= _mm256_set1_pd(isp[30]); is1= _mm256_set1_pd(isp[31]); is2= _mm256_set1_pd(isp[32]);
        u0 = _mm256_set1_pd(  u[30]); u1 = _mm256_set1_pd(  u[31]); u2 = _mm256_set1_pd(  u[32]);
        g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0), _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
        a036= _mm256_mul_pd(g0,u0); a147 = _mm256_mul_pd(g0,u1); a258 = _mm256_mul_pd(g0,u2);
            _mm256_storeu_pd(&G[30],g0);

        is3= _mm256_set1_pd(isp[33]); is4= _mm256_set1_pd(isp[34]); is5= _mm256_set1_pd(isp[35]);
        u3 = _mm256_set1_pd(  u[33]); u4 = _mm256_set1_pd(  u[34]); u5 = _mm256_set1_pd(  u[35]);
        g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3), _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
        a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
            _mm256_storeu_pd(&G[33],g1);

        is0= _mm256_set1_pd(isp[36]); is1= _mm256_set1_pd(isp[37]); is2= _mm256_set1_pd(isp[38]);
        u0 = _mm256_set1_pd(  u[36]); u1 = _mm256_set1_pd(  u[37]); u2 = _mm256_set1_pd(  u[38]);
        g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0), _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
        a036= _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
            _mm256_storeu_pd(&G[36],g0);

        is3= _mm256_set1_pd(isp[39]); is4= _mm256_set1_pd(isp[40]); is5= _mm256_set1_pd(isp[41]);
        u3 = _mm256_set1_pd(  u[39]); u4 = _mm256_set1_pd(  u[40]); u5 = _mm256_set1_pd(  u[41]);
        g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3), _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
        a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
            _mm256_storeu_pd(&G[39],g1);

        is0= _mm256_set1_pd(isp[42]); is1= _mm256_set1_pd(isp[43]); is2= _mm256_set1_pd(isp[44]);
        u0 = _mm256_set1_pd(  u[42]); u1 = _mm256_set1_pd(  u[43]); u2 = _mm256_set1_pd(  u[44]);
        g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0), _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
        a036= _mm256_mul_pd(g0,u0); a147 = _mm256_mul_pd(g0,u1); a258 = _mm256_mul_pd(g0,u2);
            _mm256_storeu_pd(&G[42],g0);

        is3= _mm256_set1_pd(isp[45]); is4= _mm256_set1_pd(isp[46]); is5= _mm256_set1_pd(isp[47]);
        u3 = _mm256_set1_pd(  u[45]); u4 = _mm256_set1_pd(  u[46]); u5 = _mm256_set1_pd(  u[47]);
        g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3), _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
        a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
        _mm256_storeu_pd(&G[45],g1);
      }
      _mm256_storeu_pd(&H[0],a036);
      _mm256_storeu_pd(&H[3],a147);
      _mm256_storeu_pd(&H[6],a258);
      }//end register scope
#else
#pragma vector unaligned
      for(int i=0; i< 9 ; i++){ H[i]=0.0; };
#pragma vector unaligned
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Nf ; k++){ G[Nf* i+k ]=0.0;
#pragma vector unaligned
          for(int j=0; j<Nd ; j++){
            G[(Nf* i+k) ] += intp_shpg[ip*Ne+ Nd* i+j ] * jac[Nd* j+k ];
          };
#pragma vector unaligned
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
#pragma vector unaligned
        for (int i=0; i<Nc; i++){
          std::memcpy(& u[Nf*i],& sysu[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); };
#ifdef FETCH_JAC
          std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj);
#endif
      }; };
#if 0
//FIXME change C indices for iso
      __m256d s048;
      __m256d w0 = _mm256_set1_pd(dw);
      s048= _mm256_mul_pd(w0, _mm256_add_pd(_mm256_mul_pd(c0,_mm256_set1_pd(H[0])), _mm256_add_pd(_mm256_mul_pd(c1,_mm256_set1_pd(H[4])), _mm256_mul_pd(c2,_mm256_set1_pd(H[8])))));
            _mm256_storeu_pd(&S[0], s048);
      S[3]=(H[1] + H[3])*C[6]*dw; // S[1]
      S[4]=(H[2] + H[6])*C[8]*dw; // S[2]
      S[5]=(H[5] + H[7])*C[7]*dw; // S[5]
#else
      S[0]=(C[0]* H[0] + C[1]* H[4] + C[1]* H[8])*dw;//Sxx
      S[4]=(C[1]* H[0] + C[0]* H[4] + C[1]* H[8])*dw;//Syy
      S[8]=(C[1]* H[0] + C[1]* H[4] + C[0]* H[8])*dw;//Szz
      //
      S[1]=( H[1] + H[3] )*C[2]*dw;// S[3]= S[1];//Sxy Syx
      S[5]=( H[5] + H[7] )*C[2]*dw;// S[7]= S[5];//Syz Szy
      S[2]=( H[2] + H[6] )*C[2]*dw;// S[6]= S[2];//Sxz Szx
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
#endif
      //------------------------------------------------------- 18+9 = 27 FLOP
#ifdef VEC_F
      {
      __m256d a036, a147, a258, g0,g1,g2;
      a036 = _mm256_loadu_pd(&S[0]); // [a3 a2 a1 a0]
      a147 = _mm256_loadu_pd(&S[3]); // [a6 a5 a4 a3]
      a258 = _mm256_loadu_pd(&S[6]); // [a9 a8 a7 a6]
      //_MM256_TRANSPOSE3_PD(a036,a147,a258);
      //__m256d s6,s7,s8;
      
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
      }
#else
#pragma vector unaligned
      for(int i=0; i<Nc; i++){
#pragma vector unaligned
        for(int k=0; k<Nf; k++){
#pragma vector unaligned
          for(int j=0; j<Nf; j++){
            f[(Nf* i+k) ] += G[(Nf* i+j) ] * S[(Nf* j+k) ];// 18*N FMA FLOP
      } } }//------------------------------------------------ N*3*6 = 18*N FLOP
#endif
#if VERB_MAX>10
      printf( "f:");
      for(int j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      } printf("\n");
#endif
    }//========================================================== end intp loop
#ifdef VEC_F
    _mm256_storeu_pd(&f[ 0],f0);
    _mm256_storeu_pd(&f[ 3],f1);
    _mm256_storeu_pd(&f[ 6],f2);
    _mm256_storeu_pd(&f[ 9],f3);
    if(elem_p>1){
    _mm256_storeu_pd(&f[12],f4);
    _mm256_storeu_pd(&f[15],f5);
    _mm256_storeu_pd(&f[18],f6);
    _mm256_storeu_pd(&f[21],f7);
    _mm256_storeu_pd(&f[24],f8);
    _mm256_storeu_pd(&f[27],f9);
    }
#pragma vector unaligned
    for(int i=0; i<Nc; i++){
#pragma vector unaligned
      for(int j=0; j<3;j++){
        sysf[3*conn[i] +j] = f[3*i +j];
    } }
#else
#pragma vector unaligned
    for (uint i=0; i<uint(Nc); i++){
      std::memcpy(& sysf[conn[i]*3],& f[Nf*i], sizeof(FLOAT_SOLV)*Nf );
    }
#endif
  }//============================================================ end elem loop
  return 0;
}
