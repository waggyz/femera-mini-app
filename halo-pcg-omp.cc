#include <utility>//std::pair
#include <vector>
#include <set>// This is ordered
#include <algorithm>    // std::copy
#include <cstring>      // std::memcpy
#include <unordered_map>
#include <vector>
#include <tuple>
#include <sstream>
#include <string>
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <omp.h>
#include "femera.h"

int PCG::BC (Mesh* ){return 1;}
int PCG::RHS(Mesh* ){return 1;}
int PCG::Iter(){return 1;}
int PCG::Solve( Elem*, Phys* ){return 1;}
//
int PCG::RHS(Elem* E, Phys* Y ){// printf("*** RHS(E,Y) ***\n");
  this->data_b=0.0;
  const uint Dn=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->rhs_vals ){ std::tie(n,f,v)=t;
    this->part_r[Dn* n+uint(f)]+= v;
  }
  return(0);
}
int PCG::BCS(Elem* E, Phys* Y ){// printf("*** BCS(E,Y) ***\n");
  uint Dn=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    //printf("FIX ID %i, DOF %i, val %+9.2e\n",i,E->bcs_vals[i].first,E->bcs_vals[i].second);
    this->part_u[Dn* n+uint(f)] = v * this->load_scal;
    if(std::abs(v) > Y->udof_magn[f]){ Y->udof_magn[f] = std::abs(v); }
    if(std::abs(v) > std::abs(this->loca_bmax[f])){ this->loca_bmax[f] = v; }
  }
  return(0);
}
int PCG::BC0(Elem* E, Phys* Y ){// printf("*** BC0(E,Y) ***\n");
  const INT_MESH Dn=(INT_MESH) Y->node_d;
  const INT_MESH Nb = this->cond_bloc_n;
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    for(INT_MESH i=0;i<Nb;i++){ this->part_d[Nb*(Dn*n+uint(f)) +i]=0.0; }
    this->part_f[Dn* n+uint(f)]=0.0;
  }
  for(auto t : E->bc0_nf   ){ std::tie(n,f)=t;
    for(INT_MESH i=0;i<Nb;i++){ this->part_d[Nb*(Dn*n+uint(f)) +i]=0.0; }
    #if VERB_MAX>10
    printf("BC0: [%i]:0\n",E->bc0_nf[i]);
    #endif
  }
  return(0);
}
int PCG::Setup( Elem* E, Phys* Y ){// printf("*** Setup(E,Y) ***\n");
  this->halo_loca_0 = E->halo_remo_n * Y->node_d;
  this->RHS( E,Y );
  this->BCS( E,Y );// Sync max Y->udof_magn before Precond()
  return(0);
}
int PCG::Init( Elem* E, Phys* Y ){// printf("*** Init(E,Y) ***\n");
#if 1
  //FIXME Move this somewhere
#if 0
#pragma omp critical
{ for(uint j=0; j<6; j++){printf("%f ",E->glob_bbox[j]); } printf("\n"); }
#pragma omp critical
{ for(uint j=0; j<4; j++){printf("%f ",this->glob_bmax[j]); } printf("\n"); }
#endif
  if(this->cube_init!=0.0){
    const INT_MESH Nn=E->node_n, Dm=E->mesh_d;
    const FLOAT_SOLV ci=this->cube_init;
    FLOAT_SOLV u[3]={1.0,-Y->mtrl_prop[1],-Y->mtrl_prop[1]};
    FLOAT_SOLV umax=0.0;
    for(int i=0; i<3; i++){
      if(std::abs(this->glob_bmax[i])>std::abs(umax)){
        umax=this->glob_bmax[i]; } }
    for(int i=0; i<3; i++){
      if(this->glob_bmax[i]==umax){ u[i]=umax; }
      else{ u[i] = -umax*Y->mtrl_prop[1]; }//FIXED Generalized for nu used.
    }
#if 0
#pragma omp critical
{ for(uint j=0; j<3; j++){printf("%f ",u[j]); } printf("\n"); }
#endif
    for(uint i=0; i<Nn; i++){
      for(uint j=0; j<Dm; j++){
        this->part_u[Dm* i+j ] = ci * u[j]
        * ( E->node_coor[Dm* i+j ] - E->glob_bbox[j] )
        / ( E->glob_bbox[   Dm+j ] - E->glob_bbox[j] );
      }
    }
  }// printf("this->BCS( E,Y )\n");
  this->BCS( E,Y );//FIXME repeated in Setup(E,Y)
  //printf("this->BC0( E,Y )\n");
  this->BC0( E,Y );
  //printf("Y->ElemLinear( E,... )\n");
#endif
  const uint sysn=this->udof_n;
  for(uint i=0; i<sysn; i++){ this->part_f[i] = 0.0; }
  Y->ElemLinear( E,0,E->elem_n,this->part_f,this->part_u );
  return 0;
}
int PCG::Init(){
  const uint sysn=this->udof_n;
  const uint sumi0=this->halo_loca_0;
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
  for(uint i=0; i<sysn; i++){
    this->part_r[i] -= this->part_f[i];
  }
  //part_r  = part_b - part_f;
  //part_z  = part_d * part_r;// This was merged where it's used (2x/iter)
  //part_p  = part_z;
  switch( this->cond_bloc_n ){
    case(3):{
      INT_MESH s=sysn/3;
      for(INT_MESH i=0; i<s; i++){
        for(INT_MESH j=0; j<3; j++){
          INT_MESH n=9*i+3*j;
          part_p[3* i+j ]
            = part_d[n    ] * part_r[3* i   ]
            + part_d[n +1 ] * part_r[3* i+1 ]
            + part_d[n +2 ] * part_r[3* i+2 ];
          //printf("%9.3f %9.3f %9.3f\n",part_d[n],part_d[n+1],part_d[n+2]);
        }
      }
    break; }
    default:{
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
      for(INT_MESH i=0; i<sysn; i++){
        part_p[i]  = part_d[i] * part_r[i];
      }
    }
  }
  FLOAT_SOLV R2=0.0;
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd reduction(+:R2)
#endif
  for(uint i=sumi0; i<sysn; i++){ R2 += part_r[i] * part_p[i]; }
  //R2 += part_r[i] * part_r[i] * part_d[i]; }
  this->loca_res2 = R2;
  this->loca_rto2 = this->loca_rtol*loca_rtol *loca_res2;//FIXME Move this somewhere.
  return(0);
}
int HaloPCG::Init(){// Preconditioned Conjugate Gradient
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  // Local copies for atomic ops and reduction
  FLOAT_SOLV glob_r2a = 0.0, glob_to2 = this->glob_rto2;
  const FLOAT_SOLV load_scal=this->step_scal * FLOAT_SOLV(this->load_step);
  Phys::vals bcmax={0.0,0.0,0.0,0.0};
  FLOAT_SOLV halo_vals[this->halo_cond_n];
#pragma omp parallel num_threads(comp_n)
{// parallel init region
  long int my_scat_count=0, my_prec_count=0,
    my_gat0_count=0,my_gat1_count=0, my_solv_count=0;
  auto start = std::chrono::high_resolution_clock::now();
#if OMP_NESTED==true
  // Make thread-local copies of mesh_part into priv_part.
  std::vector<part> priv_part;
  priv_part.resize(this->mesh_part.size());
  std::copy(this->mesh_part.begin(), this->mesh_part.end(), priv_part.begin());
#endif
  int part_0=0; if(std::get<0>( priv_part[0] )==NULL){ part_0=1; }
  const int part_n = int(priv_part.size())-part_0;
  const int part_o = part_n+part_0;
  // Sync max Y->udof_magn
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    S->load_scal=load_scal;
    for(uint i=0;i<Y->udof_magn.size();i++){
      //printf("GLOBAL MAX BC[%u]: %f\n",i,bcmax[i]);
      if(Y->udof_magn[i] > bcmax[i]){//FIXME Atomic read?
//#pragma omp atomic write
        bcmax[i]=Y->udof_magn[i];
      }
      if(std::abs(S->loca_bmax[i]) > std::abs(this->glob_bmax[i])){
        this->glob_bmax[i] = S->loca_bmax[i];
      }
    }
  }
#pragma omp single
{
  auto m=bcmax[0];
  for(uint i=1;i<3;i++){ if(bcmax[0] > m){ m=bcmax[i]; } }
  for(uint i=0;i<3;i++){ bcmax[i]=m; }
  for(uint i=0;i<bcmax.size();i++){ if(bcmax[i]<=0.0){ bcmax[i]=1.0; } }
#if VERB_MAX>2
    if(verbosity>2){
      printf("   DOF Scales: ");
      for(uint i=0;i<bcmax.size();i++){ printf(" %g",bcmax[i]); }
      printf("\n");
    }
#endif
}
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    for(uint i=0;i<Y->udof_magn.size();i++){
//#pragma omp atomic read
      Y->udof_magn[i] = bcmax[i];
      //printf("Sync MAX BC[%u]: %f\n",i,Y->udof_magn[i]);
      S->glob_bmax[i] = this->glob_bmax[i];
    }// printf("S->Precond( E,Y )...\n");
    S->Precond( E,Y );// printf("S->Precond( E,Y ) done.\n");
  }
  time_reset( my_prec_count, start );
  // ---------------------------  Sync part_d
#pragma omp single
  for(INT_MESH i=0; i<halo_cond_n; i++){ halo_vals[i]=0.0; };
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH d = (INT_MESH) Y->node_d * S->cond_bloc_n;
    if(this->solv_cond == Solv::COND_NONE){
      for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
        for( uint j=0; j<d; j++){
#pragma omp atomic write
          halo_vals[f+j] = S->part_d[d*i +j]; } }
    }else{
      for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
        for( uint j=0; j<d; j++){
#pragma omp atomic update
          halo_vals[f+j]+= S->part_d[d*i +j]; } }
    }
  }
  time_reset( my_gat1_count, start );
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH d = (INT_MESH) Y->node_d * S->cond_bloc_n;
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->part_d[d*i +j] = halo_vals[f+j];
      }
    }
  }
  time_reset( my_scat_count, start );
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){//-------------- Invert part_d
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    // Invert preconditioner
    switch( S->cond_bloc_n ){
      case(3):{
        //for(uint i=0; i<(sysn*3);i++){printf("%9.2e ",S->part_d[i]);} printf("\n");
#if 1
        // Invert 3x3 blocks. The inverse is also symmetric, so no need to transpose.
        for(INT_MESH n=0;n<E->node_n;n++){
#if 1
          E->Jac3Inv( &S->part_d[9*n],E->Jac3Det( &S->part_d[9*n] ));
#else
          FLOAT_MESH jac[9];
          for(int i=0;i<9;i++){ jac[i]=S->part_d[9*n+i]; }
          E->Jac3Inv( &jac[0], E->Jac3Det( &jac[0] ) );
          for(int i=0;i<9;i++){ S->part_d[9*n+i]=jac[i]; }
#endif
        }
#else
        // Use only the inverted diagonal for testing.
        for(INT_MESH n=0;n<E->node_n;n++){
          for(uint i=0;i<3;i++){
            for(uint j=0;j<3;j++){ auto d=9*n+3*i+j;
               //printf("%9.2e ",S->part_d[d]);
               if(i==j){S->part_d[d]=FLOAT_SOLV(1.0) / S->part_d[d];
               }else{S->part_d[d]=0.0; }
            }// printf("\n");
          }
        }
#endif
      break; }
      default:{// Diagonal preconditioner
        const INT_MESH sysn=S->udof_n;
        //for(uint i=0; i<(sysn);i++){printf("%9.2e ",S->part_d[i]);} printf("\n");
        for(uint i=0;i<sysn;i++){ S->part_d[i] = FLOAT_SOLV(1.0) / S->part_d[i]; }
      }
    }
    // Sync global bounding box.
    for(int i=0; i<6; i++){ E->glob_bbox[i]=this->glob_bbox[i]; }
    S->Init( E,Y );// Zeros boundary conditions
#if 0
    for(INT_MESH n=0;n<E->node_n;n++){
      for(uint i=0;i<3;i++){
        for(uint j=0;j<3;j++){
            printf("%9.2e ",S->part_d[9*n+3*i+j]);
        } printf("\n");
      }
    }
#endif
  }
  time_reset( my_solv_count, start );
#pragma omp single
  for(INT_MESH i=0; i<halo_cond_n; i++){ halo_vals[i]=0.0; }// serial halo_vals zero
  time_reset( my_gat0_count, start );// ---------------------------  Sync part_f
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic update
        halo_vals[f+j] += S->part_f[d*i +j]; }
    }
  }// End halo_vals
  time_reset( my_gat1_count, start );
#pragma omp for schedule(static) reduction(+:glob_r2a)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->part_f[d*i +j] = halo_vals[f+j]; }
    }
  time_reset( my_scat_count, start );// ------------------- finished part_f sync
#pragma omp critical(init)
{ S->Init(); }//FIXME Why is this serialized?
#pragma omp atomic update
  glob_r2a += S->loca_res2;
  }
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    S->loca_rto2 = S->loca_rtol*S->loca_rtol *glob_r2a;
#pragma omp atomic write
    glob_to2 = S->loca_rto2;// Pass the relative tolerance out.
  }
  time_reset( my_solv_count, start );
#if VERB_MAX>1
#pragma omp critical(time)
{
  this->time_secs[0]+=float(my_prec_count)*1e-9;
  //this->time_secs[1]+=float(my_gmap_count)*1e-9;
  this->time_secs[2]+=float(my_gat0_count)*1e-9;
  this->time_secs[3]+=float(my_gat1_count)*1e-9;
  this->time_secs[4]+=float(my_scat_count)*1e-9;
  this->time_secs[5]+=float(my_solv_count)*1e-9;
}
#endif
}// end init parallel region
  this->glob_res2 = glob_r2a;
  this->glob_chk2 = glob_r2a;
  this->glob_rto2 = glob_to2;// / ((FLOAT_SOLV)this->udof_n);
  return 0;
}
int HaloPCG::Iter(){// printf("*** Halo Iter() ***\n");
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  FLOAT_SOLV glob_sum1=0.0, glob_sum2=0.0;
  FLOAT_SOLV glob_r2a = this->glob_res2;
  FLOAT_SOLV halo_vals[this->halo_vals_n];// Put this on the stack.
#pragma omp parallel num_threads(comp_n)
{// iter parallel region
#if OMP_NESTED==true
  // Make thread-local copies of mesh_part into threadprivate HaloPCG::priv_part.
  std::vector<part> priv_part;
  priv_part.resize(this->mesh_part.size());
  std::copy(this->mesh_part.begin(), this->mesh_part.end(), priv_part.begin());
#endif
  // HaloPCG::priv_part is a threadprivate global variable
  int part_0=0; if(std::get<0>( priv_part[0] )==NULL){ part_0=1; }
  const int part_n = int(priv_part.size())-part_0;
  const int part_o = part_n+part_0;
  Elem* E; Phys* Y; Solv* S;// Seems to be faster to reuse these.
  // Timing variables (used when verbosity > 1)
  long int my_phys_count=0, my_scat_count=0, my_solv_count=0,
    my_gat0_count=0,my_gat1_count=0;
  std::chrono::high_resolution_clock::time_point iter_start,
    solv_start, inte_start, iter_done;
  std::chrono::high_resolution_clock::time_point
    gath_start, scat_start, phys_start;
  time_start( iter_start );
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    time_start( phys_start );
    const auto sysn = S->udof_n;
    for(uint i=0;i<sysn;i++){ S->part_f[i]=0.0; }
    Y->ElemLinear( E,0,E->halo_elem_n, S->part_f, S->part_p );
    time_accum( my_phys_count, phys_start );
    time_start( gath_start );
    const INT_MESH hnn=E->halo_node_n,hrn=E->halo_remo_n;
    for(INT_MESH i=hrn; i<hnn; i++){//NOTE memcpy apparently not critical
      std::memcpy(
        & halo_vals[Dn* E->node_haid[i]],
        & S->part_f[Dn* i],
        Dn*sizeof(FLOAT_PHYS) );
    }
    time_accum( my_gat0_count, gath_start );
  }
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    time_start( gath_start );
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      const auto f = Dn* E->node_haid[i];
      for( uint j=0; j<Dn; j++){
#pragma omp atomic update
        halo_vals[f+j]+= S->part_f[Dn* i+j]; }
    }
    time_accum( my_gat1_count, gath_start );
  }// End halo_vals sum; now scatter back to elems
#pragma omp for schedule(static) reduction(+:glob_sum1)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    time_start( scat_start );
    const INT_MESH hnn=E->halo_node_n,hl0=S->halo_loca_0,sysn=S->udof_n;
    for(INT_MESH i=0; i<hnn; i++){//NOTE appears not to be critical
      std::memcpy(
        & S->part_f[Dn* i],
        & halo_vals[Dn* E->node_haid[i]],
        Dn*sizeof(FLOAT_PHYS) );
    }
    time_accum( my_scat_count, scat_start );
    time_start( phys_start );
    Y->ElemLinear( E,E->halo_elem_n,E->elem_n, S->part_f, S->part_p );
    time_accum( my_phys_count, phys_start );
    time_start( solv_start );
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd reduction(+:glob_sum1)
#endif
    for(INT_MESH i=hl0; i<sysn; i++){
        glob_sum1 += S->part_p[i] * S->part_f[i];
    }
    time_accum( my_solv_count, solv_start );
  }
  time_start( solv_start );
  const FLOAT_SOLV alpha = glob_r2a / glob_sum1;// 1 FLOP
  //printf("ALPHA:%+9.2e\n",alpha);
#pragma omp for schedule(static) reduction(+:glob_sum2)
  for(int part_i=part_0; part_i<part_o; part_i++){// ? FLOP/DOF
    std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH hl0=S->halo_loca_0, sysn=S->udof_n;
    switch( S->cond_bloc_n ){
      case(3):{
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
        for(INT_MESH i=0; i<sysn; i++){
            S->part_r[i] -= S->part_f[i] * alpha; }// Update force residuals
        INT_MESH s=sysn/3;
        for(INT_MESH i=0; i<s; i++){
          for(INT_MESH j=0; j<3; j++){
            // Reuse part_f to store z = d*r.
            //NOTE Can be precon. inline function.
            INT_MESH n=9*i+3*j;
            S->part_f[3* i+j ]
              = S->part_d[n    ] * S->part_r[3* i   ]
              + S->part_d[n +1 ] * S->part_r[3* i+1 ]
              + S->part_d[n +2 ] * S->part_r[3* i+2 ];
            //glob_sum2 += S->part_r[3* i+j ] * S->part_f[3* i+j ]
            //  *(FLOAT_SOLV( (3* i+j)>=hl0 ));
          }
        }
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd reduction(+:glob_sum2)
#endif
        for(INT_MESH i=hl0; i<sysn; i++){
          glob_sum2    += S->part_r[i] * S->part_f[i]; }
      break; }
      default:{// Diagonal
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd reduction(+:glob_sum2)
#endif
        for(INT_MESH i=0; i<sysn; i++){
          S->part_r[i] -= S->part_f[i] * alpha;// Update force residuals
          // Reuse part_f to store z = d*r.
          S->part_f[i]  = S->part_d[i] * S->part_r[i];
          glob_sum2    += S->part_r[i] * S->part_f[i] *(FLOAT_SOLV( i>=hl0 ));
        }
      }
    }
  }
  const FLOAT_PHYS beta = glob_sum2 / glob_r2a;// 1 FLOP
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH sysn=S->udof_n;
    //part_p  = part_d * part_r + (r2b/ra)*part_p;
    //S->r2a = glob_sum2;// Update member residual (squared)
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(INT_MESH i=0; i<sysn; i++){// ? FLOP/DOF
      S->part_u[i] += S->part_p[i] * alpha;// better data locality here
      // Reuse part_f to store z = d*r.
      S->part_p[i]  = S->part_f[i] + S->part_p[i] * beta;
    }
  }
#if VERB_MAX>1
  iter_done  = std::chrono::high_resolution_clock::now();
  auto solv_time  = std::chrono::duration_cast<std::chrono::nanoseconds>
    (iter_done - solv_start);
  auto iter_time  = std::chrono::duration_cast<std::chrono::nanoseconds>
    (iter_done - iter_start);// printf("%i ",iter);
#pragma omp critical(time)
{
  this->time_secs[0]+=float(my_phys_count)*1e-9;
  this->time_secs[1]+=float(my_gat0_count)*1e-9;
  this->time_secs[2]+=float(my_gat1_count)*1e-9;
  this->time_secs[3]+=float(my_scat_count)*1e-9;
  this->time_secs[4]+=float(my_solv_count+solv_time.count())*1e-9;
 //this->time_secs[4]+=float(solv_time.count())*1e-9;
  this->time_secs[5]+=float(iter_time.count())*1e-9;
}
#endif
}// end iter parallel region
  this->glob_res2 = glob_sum2;
  this->glob_chk2 = glob_sum2;
  return 0;
}
