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
int ElastOrtho3D::ScatStiff( Elem* ){ return 1; };//FIXME
//
int ElastOrtho3D::Setup( Elem* E ){
    JacRot( E );
    const uint jacs_n = E->elip_jacs.size()/E->elem_n/ 10 ;
    const uint intp_n = E->gaus_n;
    this->tens_flop = uint(E->elem_n) * intp_n
        *( uint(E->elem_conn_n)* (36+18+3) + 2*54 + 27 );
    this->tens_band = uint(E->elem_n) *(
            sizeof(FLOAT_PHYS)*(3*uint(E->elem_conn_n)*3+ jacs_n*10)
            +sizeof(INT_MESH)*uint(E->elem_conn_n) );
    this->stif_flop = uint(E->elem_n)
        * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
    this->stif_band = uint(E->elem_n) *(
            sizeof(FLOAT_PHYS)* 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) -1+3)
            +sizeof(INT_MESH) *uint(E->elem_conn_n) );
    return 0;
};
int ElastOrtho3D::ElemLinear( Elem* E, RESTRICT Phys::vals &sys_f, const RESTRICT Phys::vals &sys_u ){
    //FIXME Cleanup local variables.
    const int ndof   = 3;//this->ndof_n
    //const int mesh_d = 3;//E->elem_d;
    const int  Nj =10;//,d2=9;//mesh_d*mesh_d;
    //
    const INT_MESH elem_n = E->elem_n;
    const int     Nc = E->elem_conn_n;// Number of Nodes/Element
    const int     Ne = ndof*Nc;
    const int intp_n = int(E->gaus_n);
    //
    INT_MESH e0=0, ee=elem_n;
    if(E->do_halo==true){ e0=0; ee=E->halo_elem_n;
    }else{ e0=E->halo_elem_n; ee=elem_n; };
    //
#if VERB_MAX>11
    printf("Elems:%i, IntPts:%i, Nodes/elem:%i\n",
            (int)elem_n,(int)intp_n,(int)Nc);
    //(int)mesh_d,(int)elem_n,(int)intp_n,(int)Nc);
#endif
    //INT_MESH   conn[Nc];
    FLOAT_MESH jac[Nj];//, det;
    FLOAT_PHYS dw, G[Ne+1], u[Ne],f[Ne+1];
    FLOAT_PHYS H[9+1], S[9], A[9+1];//, B[9];
    //
    FLOAT_PHYS intp_shpg[intp_n*Ne];
    std::copy( &E->intp_shpg[0],// local copy
            &E->intp_shpg[intp_n*Ne], intp_shpg );
    FLOAT_PHYS wgt[intp_n];
    std::copy( &E->gaus_weig[0],
            &E->gaus_weig[intp_n], wgt );
    FLOAT_PHYS C[this->mtrl_matc.size()];
    std::copy( &this->mtrl_matc[0], &this->mtrl_matc[this->mtrl_matc.size()], C );
    const FLOAT_PHYS R[9] = {
        mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
        mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
        mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8]};

    const FLOAT_PHYS Rt[9+1] = {
        mtrl_rotc[0],mtrl_rotc[3],mtrl_rotc[6],
        mtrl_rotc[1],mtrl_rotc[4],mtrl_rotc[7],
        mtrl_rotc[2],mtrl_rotc[5],mtrl_rotc[8],0.0};
#if VERB_MAX>10
    printf( "Material [%u]:", (uint)mtrl_matc.size() );
    for(uint j=0;j<mtrl_matc.size();j++){
        //if(j%mesh_d==0){printf("\n");}
        printf("%+9.2e ",C[j]);
    }; printf("\n");
#endif
    const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
    const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
    const FLOAT_SOLV* RESTRICT sysu0 = &sys_u[0];
    if(e0<ee){
        //std::memcpy( &conn, &Econn[Nc*e0], sizeof(  INT_MESH)*Nc);
        std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
        const   INT_MESH* RESTRICT c = &Econn[Nc*e0];
        for (uint i=0; i<Nc; i++){
            std::memcpy( &    u[ndof*i], &sysu0[c[i]*ndof], sizeof(FLOAT_SOLV)*ndof );
        };
    };
    //bool fetch_next=false;

    for(INT_MESH ie=e0;ie<ee;ie++){
        //if((ie+1)<ee){fetch_next=true;}else{fetch_next=false;};
        //const   INT_MESH* RESTRICT conn = &Econn[Nc*ie];
        //const FLOAT_MESH* RESTRICT jac  = &Ejacs[Nj*ie];
        //std::memcpy( &conn, &Econn[Nc*ie], sizeof(  INT_MESH)*Nc);
        //std::memcpy( &jac , &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
        //std::copy( &Econn[Nc*ie],
        //           &Econn[Nc*ie+Nc], conn );
        //std::copy( &Ejacs[Nj*ie],
        //           &Ejacs[Nj*ie+Nj], jac );// det=jac[9];
        //for (uint i=0; i<(Nc); i++){
        //  //std::memcpy( &    u[ndof*i],
        //  std::memcpy( &    u[ndof*i],
        //               //&sys_u[Econn[Nc*ie+i]*ndof], sizeof(FLOAT_SOLV)*ndof ); };
        //               &sysu0[conn[i]*ndof], sizeof(FLOAT_SOLV)*ndof ); };

        //for(int i=0;i<Ne;i++){ f[i]=0.0; };
#if 1
        __m256d f0,f1,f2,f3,f4,f5,f6,f7,f8,f9;
        const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
        f0 = _mm256_loadu_pd(&sys_f[3*conn[0]]); f1 = _mm256_loadu_pd(&sys_f[3*conn[1]]); f2 = _mm256_loadu_pd(&sys_f[3*conn[2]]);
        f3 = _mm256_loadu_pd(&sys_f[3*conn[3]]); f4 = _mm256_loadu_pd(&sys_f[3*conn[4]]); f5 = _mm256_loadu_pd(&sys_f[3*conn[5]]);
        f6 = _mm256_loadu_pd(&sys_f[3*conn[6]]); f7 = _mm256_loadu_pd(&sys_f[3*conn[7]]); f8 = _mm256_loadu_pd(&sys_f[3*conn[8]]);
        f9 = _mm256_loadu_pd(&sys_f[3*conn[9]]);
#else
        for(int i=0;i<Ne;i++){ f[i]=0.0; };
#endif
        __m256d j0,j1,j2;
        j0 = _mm256_loadu_pd(&jac[0]);  // j0 = [j3 j2 j1 j0]
        j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
        j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]

    for(int ip=0; ip<intp_n; ip++){
        //G = MatMul3x3xN( jac,shg );
        //A = MatMul3xNx3T( G,u );
        //for(uint i=0; i<(Ne) ; i++){ G[i]=0.0; };

        __m256d s0,s1,s2,s3,s4,s5;
        __m256d u0,u1,u2,u3,u4,u5,g0,g1,g2;
        __m256d is0,is1,is2,is3,is4,is5;
        __m256d a036, a147, a258;
        double * RESTRICT isp = &intp_shpg[ip*Ne];

        is0= _mm256_set1_pd(isp[0]); is1= _mm256_set1_pd(isp[1]); is2= _mm256_set1_pd(isp[2]);
        u0 = _mm256_set1_pd(  u[0]); u1 = _mm256_set1_pd(  u[1]); u2 = _mm256_set1_pd(  u[2]);
        s0 = _mm256_mul_pd(j0,is0) ; s1 = _mm256_mul_pd(j1,is1) ; s2 = _mm256_mul_pd(j2,is2);
        g0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
        a036 = _mm256_mul_pd(g0,u0); a147 = _mm256_mul_pd(g0,u1); a258 = _mm256_mul_pd(g0,u2);
        _mm256_storeu_pd(&G[0],g0);

        is3= _mm256_set1_pd(isp[3]); is4= _mm256_set1_pd(isp[4]); is5= _mm256_set1_pd(isp[5]);
        u3 = _mm256_set1_pd(  u[3]); u4 = _mm256_set1_pd(  u[4]); u5 = _mm256_set1_pd(  u[5]);
        s3 = _mm256_mul_pd(j0,is3) ; s4 = _mm256_mul_pd(j1,is4) ; s5 = _mm256_mul_pd(j2,is5);
        g1 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
        a036 = _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
        _mm256_storeu_pd(&G[3],g1);

        is0= _mm256_set1_pd(isp[6]); is1= _mm256_set1_pd(isp[7]); is2= _mm256_set1_pd(isp[8]);
        u0 = _mm256_set1_pd(  u[6]); u1 = _mm256_set1_pd(  u[7]); u2 = _mm256_set1_pd(  u[8]);
        s0 = _mm256_mul_pd(j0,is0) ; s1 = _mm256_mul_pd(j1,is1) ; s2 = _mm256_mul_pd(j2,is2);
        g0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
        a036 = _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
        _mm256_storeu_pd(&G[6],g0);

        is3= _mm256_set1_pd(isp[9]); is4= _mm256_set1_pd(isp[10]); is5= _mm256_set1_pd(isp[11]);
        u3 = _mm256_set1_pd(  u[9]); u4 = _mm256_set1_pd(  u[10]); u5 = _mm256_set1_pd(  u[11]);
        s3 = _mm256_mul_pd(j0,is3) ; s4 = _mm256_mul_pd(j1,is4)  ; s5 = _mm256_mul_pd(j2,is5);
        g1 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
        a036 = _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
        _mm256_storeu_pd(&G[9],g1);

        is0= _mm256_set1_pd(isp[12]); is1= _mm256_set1_pd(isp[13]); is2= _mm256_set1_pd(isp[14]);
        u0 = _mm256_set1_pd(  u[12]); u1 = _mm256_set1_pd(  u[13]); u2 = _mm256_set1_pd(  u[14]);
        s0 = _mm256_mul_pd(j0,is0)  ; s1 = _mm256_mul_pd(j1,is1)  ; s2 = _mm256_mul_pd(j2,is2);
        g0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
        a036 = _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
        _mm256_storeu_pd(&G[12],g0);

        is3= _mm256_set1_pd(isp[15]); is4= _mm256_set1_pd(isp[16]); is5= _mm256_set1_pd(isp[17]);
        u3 = _mm256_set1_pd(  u[15]); u4 = _mm256_set1_pd(  u[16]); u5 = _mm256_set1_pd(  u[17]);
        s3 = _mm256_mul_pd(j0,is3)  ; s4 = _mm256_mul_pd(j1,is4)  ; s5 = _mm256_mul_pd(j2,is5);
        g1 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
        a036 = _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
        _mm256_storeu_pd(&G[15],g1);

        is0= _mm256_set1_pd(isp[18]); is1= _mm256_set1_pd(isp[19]); is2= _mm256_set1_pd(isp[20]);
        u0 = _mm256_set1_pd(  u[18]); u1 = _mm256_set1_pd(  u[19]); u2 = _mm256_set1_pd(  u[20]);
        s0 = _mm256_mul_pd(j0,is0)  ; s1 = _mm256_mul_pd(j1,is1)  ; s2 = _mm256_mul_pd(j2,is2);
        g0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
        a036 = _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
        _mm256_storeu_pd(&G[18],g0);

        is3= _mm256_set1_pd(isp[21]); is4= _mm256_set1_pd(isp[22]); is5= _mm256_set1_pd(isp[23]);
        u3 = _mm256_set1_pd(  u[21]); u4 = _mm256_set1_pd(  u[22]); u5 = _mm256_set1_pd(  u[23]);
        s3 = _mm256_mul_pd(j0,is3)  ; s4 = _mm256_mul_pd(j1,is4)  ; s5 = _mm256_mul_pd(j2,is5);
        g1 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
        a036 = _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
        _mm256_storeu_pd(&G[21],g1);

        is0= _mm256_set1_pd(isp[24]); is1= _mm256_set1_pd(isp[25]); is2= _mm256_set1_pd(isp[26]);
        u0 = _mm256_set1_pd(  u[24]); u1 = _mm256_set1_pd(  u[25]); u2 = _mm256_set1_pd(  u[26]);
        s0 = _mm256_mul_pd(j0,is0)  ; s1 = _mm256_mul_pd(j1,is1)  ; s2 = _mm256_mul_pd(j2,is2);
        g0 = _mm256_add_pd(s0, _mm256_add_pd(s1,s2));
        a036 = _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
        _mm256_storeu_pd(&G[24],g0);

        is3= _mm256_set1_pd(isp[27]); is4= _mm256_set1_pd(isp[28]); is5= _mm256_set1_pd(isp[29]);
        u3 = _mm256_set1_pd(  u[27]); u4 = _mm256_set1_pd(  u[28]); u5 = _mm256_set1_pd(  u[29]);
        s3 = _mm256_mul_pd(j0,is3)  ; s4 = _mm256_mul_pd(j1,is4)  ; s5 = _mm256_mul_pd(j2,is5);
        g1 = _mm256_add_pd(s3, _mm256_add_pd(s4,s5));
        a036 = _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
        _mm256_storeu_pd(&G[27],g1);

        _MM256_TRANSPOSE3_PD(a036,a147,a258);
        _mm256_storeu_pd(&A[0],a036); // [?? a2 a1 a0]
        _mm256_storeu_pd(&A[3],a147); // [?? a5 a4 a3]
        _mm256_storeu_pd(&A[6],a258); // [?? a8 a7 a6]

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
            for (int i=0; i<Nc; i++){
                for(int j=0; j<3 ; j++){}
                std::memcpy( &u[ndof*i], &sysu0[c[i]*ndof], sizeof(FLOAT_SOLV)*ndof );
            };
        }; };
        // [H] Small deformation tensor
        // [H][RT] : matmul3x3x3T

          H[0]  = A[0]*R[0] + A[1]*R[1] + A[2]*R[2];
          H[1]  = A[0]*R[3] + A[1]*R[4] + A[2]*R[5];
          H[2]  = A[0]*R[6] + A[1]*R[7] + A[2]*R[8];

          H[3]  = A[3]*R[0] + A[4]*R[1] + A[5]*R[2];
          H[4]  = A[3]*R[3] + A[4]*R[4] + A[5]*R[5];
          H[5]  = A[3]*R[6] + A[4]*R[7] + A[5]*R[8];

          H[6]  = A[6]*R[0] + A[7]*R[1] + A[8]*R[2];
          H[7]  = A[6]*R[3] + A[7]*R[4] + A[8]*R[5];
          H[8]  = A[6]*R[6] + A[7]*R[7] + A[8]*R[8];

        //
        S[0]=(C[0]*H[0] + C[3]*H[4] + C[5]*H[8])*dw; S[1]=(H[1] + H[3])*C[6]*dw;                  S[2]=(H[2] + H[6])*C[8]*dw;
                                                     S[4]=(C[3]*H[0] + C[1]*H[4] + C[4]*H[8])*dw; S[5]=(H[5] + H[7])*C[7]*dw;
                                                                                                  S[8]=(C[5]*H[0] + C[4]*H[4] + C[2]*H[8])*dw;
        //
#if VERB_MAX>12
        printf( "Stress (Natural Coords):");
        for(uint j=0;j<9;j++){
            if(j%3==0){printf("\n");}
            printf("%+9.2e ",S[j]);
        }; printf("\n");
#endif
        //--------------------------------------------------------- 18+9= 27 FLOP
        // [S][R] : matmul3x3x3, R is transposed
        //for(int i=0; i<9; i++){ A[i]=0.0; };

        __m256d a0,a1,a2,a3,a4,a5,a6,a7,a8;
        __m256d r0,r3,r6,s6,s7,s8;
        r0  = _mm256_loadu_pd(&R[0]); r3 = _mm256_loadu_pd(&R[3]); r6 = _mm256_loadu_pd(&R[6]);
        s0  = _mm256_set1_pd(S[0]) ; s1 = _mm256_set1_pd(S[1]) ; s2 = _mm256_set1_pd(S[2]);
        a0  = _mm256_mul_pd(r0,s0) ; a1 = _mm256_mul_pd(r3,s1) ; a2 = _mm256_mul_pd(r6,s2);
        a036=_mm256_add_pd(a0, _mm256_add_pd(a1,a2));

                                    s4 = _mm256_set1_pd(S[4]); s5 = _mm256_set1_pd(S[5]);
        a3  = _mm256_mul_pd(r0,s1); a4 = _mm256_mul_pd(r3,s4); a5 = _mm256_mul_pd(r6,s5);
        a147=_mm256_add_pd(a3, _mm256_add_pd(a4,a5));

                                                               s8 = _mm256_set1_pd(S[8]);
        a6  = _mm256_mul_pd(r0,s2); a7 = _mm256_mul_pd(r3,s5); a8 = _mm256_mul_pd(r6,s8);
        a258=_mm256_add_pd(a6, _mm256_add_pd(a7,a8));

        //NOTE [A] is not symmetric Cauchy stress.
        //NOTE Cauchy stress is ( A + AT ) /2
#if VERB_MAX>12
        printf( "Rotated Stress (Global Coords):");
        for(uint j=0;j<9;j++){
            if(j%3==0){printf("\n");}
            printf("%+9.2e ",S[j]);
        }; printf("\n");
#endif

        __m256d g3,g4,g5,g6,g7,g8,g9,g10,g11;
        __m256d s9,s10,s11;
        g0 = _mm256_set1_pd(G[0])  ; g1 = _mm256_set1_pd(G[1])  ; g2 = _mm256_set1_pd(G[2]);
        s0 = _mm256_mul_pd(g0,a036); s1 = _mm256_mul_pd(g1,a147); s2 = _mm256_mul_pd(g2,a258);
        f0 = _mm256_add_pd(f0, _mm256_add_pd(s0, _mm256_add_pd(s1,s2)));

        g3 = _mm256_set1_pd(G[3])  ; g4 = _mm256_set1_pd(G[4])  ; g5 = _mm256_set1_pd(G[5]);
        s3 = _mm256_mul_pd(g3,a036); s4 = _mm256_mul_pd(g4,a147); s5 = _mm256_mul_pd(g5,a258);
        f1 = _mm256_add_pd(f1, _mm256_add_pd(s3, _mm256_add_pd(s4,s5)));

        g6 = _mm256_set1_pd(G[6])  ; g7 = _mm256_set1_pd(G[7])  ; g8 = _mm256_set1_pd(G[8]);
        s6 = _mm256_mul_pd(g6,a036); s7 = _mm256_mul_pd(g7,a147); s8 = _mm256_mul_pd(g8,a258);
        f2 = _mm256_add_pd(f2, _mm256_add_pd(s6, _mm256_add_pd(s7,s8)));

        g9 = _mm256_set1_pd(G[9])  ; g10= _mm256_set1_pd(G[10])  ; g11 = _mm256_set1_pd(G[11]);
        s9 = _mm256_mul_pd(g9,a036); s10= _mm256_mul_pd(g10,a147); s11 = _mm256_mul_pd(g11,a258);
        f3 = _mm256_add_pd(f3, _mm256_add_pd(s9, _mm256_add_pd(s10,s11)));

        g0 = _mm256_set1_pd(G[12]) ; g1 = _mm256_set1_pd(G[13]) ; g2 = _mm256_set1_pd(G[14]);
        s0 = _mm256_mul_pd(g0,a036); s1 = _mm256_mul_pd(g1,a147); s2 = _mm256_mul_pd(g2,a258);
        f4 = _mm256_add_pd(f4, _mm256_add_pd(s0, _mm256_add_pd(s1,s2)));

        g3 = _mm256_set1_pd(G[15]) ; g4 = _mm256_set1_pd(G[16]) ; g5 = _mm256_set1_pd(G[17]);
        s3 = _mm256_mul_pd(g3,a036); s4 = _mm256_mul_pd(g4,a147); s5 = _mm256_mul_pd(g5,a258);
        f5 = _mm256_add_pd(f5, _mm256_add_pd(s3, _mm256_add_pd(s4,s5)));

        g6 = _mm256_set1_pd(G[18]) ; g7 = _mm256_set1_pd(G[19]) ; g8 = _mm256_set1_pd(G[20]);
        s6 = _mm256_mul_pd(g6,a036); s7 = _mm256_mul_pd(g7,a147); s8 = _mm256_mul_pd(g8,a258);
        f6 = _mm256_add_pd(f6, _mm256_add_pd(s6, _mm256_add_pd(s7,s8)));

        g9 = _mm256_set1_pd(G[21]) ; g10= _mm256_set1_pd(G[22])  ; g11 = _mm256_set1_pd(G[23]);
        s9 = _mm256_mul_pd(g9,a036); s10= _mm256_mul_pd(g10,a147); s11 = _mm256_mul_pd(g11,a258);
        f7 = _mm256_add_pd(f7, _mm256_add_pd(s9, _mm256_add_pd(s10,s11)));

        g0 = _mm256_set1_pd(G[24]) ; g1 = _mm256_set1_pd(G[25]) ; g2 = _mm256_set1_pd(G[26]);
        s0 = _mm256_mul_pd(g0,a036); s1 = _mm256_mul_pd(g1,a147); s2 = _mm256_mul_pd(g2,a258);
        f8 = _mm256_add_pd(f8, _mm256_add_pd(s0, _mm256_add_pd(s1,s2)));

        g3 = _mm256_set1_pd(G[27]) ; g4 = _mm256_set1_pd(G[28]) ; g5 = _mm256_set1_pd(G[29]);
        s3 = _mm256_mul_pd(g3,a036); s4 = _mm256_mul_pd(g4,a147); s5 = _mm256_mul_pd(g5,a258);
        f9 = _mm256_add_pd(f9, _mm256_add_pd(s3, _mm256_add_pd(s4,s5)));

        // This is way slower:
        //for(uint i=0; i<Nc; i++){
        //  for(uint k=0; k<3 ; k++){
        //    for(uint j=0; j<3 ; j++){
        //    for(uint l=0; l<3 ; l++){
        //      f[3* i+l ] += G[3* i+j ] * S[3* j+k ] * R[3* k+l ];
        //};};};};
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
        //const   INT_MESH* RESTRICT conn = &Econn[Nc*ie];
#pragma vector unaligned
        for (int i=0; i<Nc; i++){
#pragma vector unaligned
            for(int j=0; j<3; j++){
                //sys_f[3*Econn[Nc*ie+i]+j] += f[(3*i+j)];
                sys_f[3*conn[i]+j] = f[(3*i+j)];
            }; };//--------------------------------------------------- N*3 =  3*N FLOP
        //if(fetch_next){
        //  std::memcpy( &conn, &Econn[Nc*(ie+1)], sizeof(  INT_MESH)*Nc); };
};//end elem loop
return 0;
};
int ElastOrtho3D::BlocLinear( Elem* E, RESTRICT Phys::vals &sys_f, const RESTRICT Phys::vals &sys_u ){
    //FIXME Cleanup local variables.
    const uint ndof   = 3;//this->ndof_n
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
    printf("Elems:%i, IntPts:%i, Nodes/elem:%i\n",
            (int)elem_n,(int)intp_n,(int)Nc);
    //printf("Dim: %i, Elems:%i, IntPts:%i, Nodes/elem:%i\n",
    //(int)mesh_d,(int)elem_n,(int)intp_n,(int)Nc);
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
                        G[(3* k+i)*Nv+l ] += jac[3* i+j +l*Nj ] * intp_shpg[ip*Ne+ 3* k+j ];
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
#if VERB_MAX>12
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
#if VERB_MAX>12
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
    const uint ndof   = 3;//this->ndof_n
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
    // Does the following wrong one actually converge faster?
    //mtrl_matc[0],mtrl_matc[1],mtrl_matc[1],0.0,0.0,0.0,
    //mtrl_matc[1],mtrl_matc[0],mtrl_matc[1],0.0,0.0,0.0,
    //mtrl_matc[1],mtrl_matc[1],mtrl_matc[0],0.0,0.0,0.0,
    //0.0,0.0,0.0,mtrl_matc[2],0.0,0.0,
    //0.0,0.0,0.0,0.0,mtrl_matc[2],0.0,
    //0.0,0.0,0.0,0.0,0.0,mtrl_matc[2]};
    //elem_inout=0.0;
for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;
    std::copy( &E->elip_jacs[ij],
            &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint i=0;i<Ne;i++){ elem_diag[i]=0.0; };
    for(uint ip=0;ip<intp_n;ip++){
        //uint ij=Nj*ie*intp_n +Nj*ip;
        //std::copy( &E->elip_jacs[ij],
        //           &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
        //jac = E->elip_jacs[std::slice(ie*intp_n*Nj+ip*Nj,d2,1)];
        //det = E->elip_jacs[ie*intp_n*Nj+ip*Nj +d2];
        //shg = E->intp_shpg[std::slice(ip*Ne,Ne,1)];
        //G   = MatMul3x3xN(jac,shg);
        uint ig=ip*Ne;
        for(uint i=0;i<Ne;i++){ G[i]=0.0; };
        for(uint k=0;k<Nc;k++){
            for(uint i=0;i<3;i++){
                for(uint j=0;j<3;j++){
                    //G[3* i+k] += jac[3* i+j] * E->intp_shpg[ig+Nc* j+k]; }; }; };
#if 1
                    G[3* i+k] += jac[3*j+i] * E->intp_shpg[ig+3* k+j ]; }; }; };
#else
                    G[3* i+k] += jac[3* i+j ] * E->intp_shpg[ig+3* k+j ]; }; }; };
#endif
#if VERB_MAX>12
printf( "Jacobian Inverse & Determinant:");
for(uint j=0;j<d2;j++){
    if(j%3==0){printf("\n");}
    printf("%+9.2e",jac[j]);
}; printf(" det:%+9.2e\n",det);
#endif
// xx yy zz
//B[std::slice(Ne*0 + 0,Nc,ndof)] = G[std::slice(Nc*0,Nc,1)];
//B[std::slice(Ne*1 + 1,Nc,ndof)] = G[std::slice(Nc*1,Nc,1)];
//B[std::slice(Ne*2 + 2,Nc,ndof)] = G[std::slice(Nc*2,Nc,1)];
for(uint j=0; j<Nc; j++){
    B[Ne*0 + 0+j*ndof] = G[Nc*0+j];
    B[Ne*1 + 1+j*ndof] = G[Nc*1+j];
    B[Ne*2 + 2+j*ndof] = G[Nc*2+j];
    // xy yx
    //B[std::slice(Ne*3 + 0,Nc,ndof)] = G[std::slice(Nc*1,Nc,1)];
    //B[std::slice(Ne*3 + 1,Nc,ndof)] = G[std::slice(Nc*0,Nc,1)];
    B[Ne*3 + 0+j*ndof] = G[Nc*1+j];
    B[Ne*3 + 1+j*ndof] = G[Nc*0+j];
    // yz zy
    //B[std::slice(Ne*4 + 1,Nc,ndof)] = G[std::slice(Nc*2,Nc,1)];
    //B[std::slice(Ne*4 + 2,Nc,ndof)] = G[std::slice(Nc*1,Nc,1)];
    B[Ne*4 + 1+j*ndof] = G[Nc*2+j];
    B[Ne*4 + 2+j*ndof] = G[Nc*1+j];
    // xz zx
    //B[std::slice(Ne*5 + 0,Nc,ndof)] = G[std::slice(Nc*2,Nc,1)];
    //B[std::slice(Ne*5 + 2,Nc,ndof)] = G[std::slice(Nc*0,Nc,1)];
    B[Ne*5 + 0+j*ndof] = G[Nc*2+j];
    B[Ne*5 + 2+j*ndof] = G[Nc*0+j];
};
#if VERB_MAX>12
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
    const uint ndof   = 3;//this->ndof_n
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
                        G[3* i+k] += jac[3* i+j] * E->intp_shpg[ig+3* k+j]; }; }; };
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
#if VERB_MAX>12
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
    const uint ndof= 3;//this->ndof_n
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
                        G[(3* k+i) ] += jac[3* i+j ] * intp_shpg[ip*Ne+ 3* k+j ];
                    };
                };
            };//------------------------------------------------- N*3*6*2 = 36*N FLOP
#if VERB_MAX>12
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
