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
//
#undef HALO_SUM_SER
#undef HALO_SUM_CHK
//
int NCG::BC (Mesh* ){return 1;}
int NCG::RHS(Mesh* ){return 1;}
int NCG::Iter(){return 1;}
int NCG::Solve( Elem*, Phys* ){return 1;}
//
int NCG::RHS(Elem* E, Phys* Y ){// printf("*** NCG::RHS(E,Y) ***\n");
  this->data_b=0.0;
  const uint Dn=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->rhs_vals ){ std::tie(n,f,v)=t;
    this->part_b[Dn* n+uint(f)]+= v;
  }
  return(0);
}
int NCG::BCS(Elem* E, Phys* Y ){
  //printf("*** NCG::BCS(E,Y) scale: %f ***\n",this->load_scal);
  uint Dn=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    //printf("FIX ID %i, DOF %i, val %+9.2e\n",
    //  i,E->bcs_vals[i].first,E->bcs_vals[i].second);
    this->part_u[Dn* n+uint(f)] = v * this->load_scal;
    if(std::abs(v) > Y->udof_magn[f]){ Y->udof_magn[f] = std::abs(v); }
    if(std::abs(v) > std::abs(this->loca_bmax[f])){ this->loca_bmax[f] = v; }
  }
  return(0);
}
int NCG::BC0(Elem* E, Phys* Y ){// printf("*** NCG::BC0(E,Y) ***\n");
  uint Dn=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    this->part_1[Dn* n+uint(f)]=1.0;//FIXME replace w/ 1.0-part_0?
    this->part_d[Dn* n+uint(f)]=0.0;
    this->part_0[Dn* n+uint(f)]=0.0;
    //this->part_f[Dn* n+uint(f)]=0.0;//FIXME apply to part_b?
  }
  for(auto t : E->bc0_nf   ){ std::tie(n,f)=t;
    this->part_1[Dn* n+uint(f)]=1.0;
    this->part_d[Dn* n+uint(f)]=0.0;
    this->part_0[Dn* n+uint(f)]=0.0;
    #if VERB_MAX>10
    printf("BC0: [%i]:0\n",E->bc0_nf[i]);
    #endif
  }
  return(0);
}
int NCG::Setup( Elem* E, Phys* Y ){// printf("*** NCG::Setup(E,Y) ***\n");
  this->halo_loca_0 = E->halo_remo_n * Y->node_d;
#if 1
  this->RHS( E,Y );
  //this->BCS( E,Y );// Sync max Y->udof_magn before Precond()
  if(this->cube_init!=0.0){//FIXME I don't know if this is working
    // printf("*** NCG::Init(E,Y)...cube_init ***\n");
    const INT_MESH Nn=E->node_n, Dm=E->mesh_d;
    const FLOAT_SOLV ci=this->cube_init;
    FLOAT_SOLV u[3]={1.0,-0.3,-0.3};
    FLOAT_SOLV umax=0.0;
    for(int i=0; i<3; i++){
      if(std::abs(this->glob_bmax[i])>std::abs(umax)){
        umax=this->glob_bmax[i]; } }
    for(int i=0; i<3; i++){
      if(this->glob_bmax[i]==umax){ u[i]=umax; }
      else{ u[i] = -umax*0.3; }//FIXME Generalize for nu used.
    }
    for(uint i=0; i<Nn; i++){
      for(uint j=0; j<Dm; j++){
        this->part_u[Dm* i+j ] = ci * u[j]
          * ( E->node_coor[Dm* i+j ] - E->glob_bbox[j] )
          / ( E->glob_bbox[   Dm+j ] - E->glob_bbox[j] );
      }
    }
  }
  Y->tens_flop*=2;Y->tens_band*=2;Y->stif_flop*=2;Y->stif_band*=2;
  //FIXME 2 evals/iter, but only some have plasticity calcs...
  this->data_f=0.0;
  Y->ElemLinear( E,0,E->elem_n, this->part_f, this->part_u );
#endif
  return(0);
}
int NCG::Init( Elem* E, Phys* Y ){//FIXME what is the point of this method?
  // printf("*** NCG::Init(E,Y) ***\n");
  this->BCS( E,Y );//FIXME repeated in Setup(E,Y)
  this->BC0( E,Y );
  return 0;
}
int NCG::Init(){// printf("*** NCG::Init() ***\n");
  return(0);
}
int HaloNCG::Init(){// printf("*** HaloNCG::Init() ***\n");
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  // Local copies for atomic ops and reduction
  //FLOAT_SOLV glob_r2a = this->glob_res2, glob_to2 = this->glob_rto2;
  FLOAT_SOLV glob_r2a = 0.0, glob_to2 = this->glob_rto2;
  Phys::vals bcmax={0.0,0.0,0.0,0.0};
#pragma omp parallel num_threads(comp_n)
{// parallel init region
  long int my_scat_count=0, my_prec_count=0,//FIXME These times are messed up
    my_gat0_count=0,my_gat1_count=0, my_solv_count=0;
  auto start = std::chrono::high_resolution_clock::now();
#if 0
  // Make thread-local copies of mesh_part into threadprivate priv_part.
  priv_part.resize(this->mesh_part.size());
  std::copy(this->mesh_part.begin(), this->mesh_part.end(), priv_part.begin());
#endif
  int part_0=0; if(std::get<0>( priv_part[0] )==NULL){ part_0=1; }
  const int part_n = int(priv_part.size())-part_0;
  const int part_o = part_n+part_0;
  // Sync max Y->udof_magn
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const uint sysn=S->udof_n;
#ifdef HAS_SIMD
#pragma omp simd
#endif
    for(INT_MESH i=1; i<sysn; i++){ S->part_f[i]=0.0; }
    // Initialize state variables.
    Y->ElemNonlinear( E,0,E->elem_n,S->part_f,S->part_u,S->part_u, true );
    if( this->next_scal > 0.0 ){
      // Predict next solution
      //FIXME Better to use prev_u + 1/step_n * tangent modulus (only) response?
      //const INT_MESH sysn=S->udof_n;
#ifdef HAS_SIMD
#pragma omp simd
#endif
      for(uint i=0;i<sysn;i++){
        const FLOAT_PHYS s = this->next_scal;
        const FLOAT_PHYS s1= 1.0+s;
        auto u=S->part_u[i];
        //S->part_u[i] = 2.0*S->part_u[i] - S->prev_u[i];
        //S->part_u[i] = S->part_u[i] + s*(S->part_u[i]- S->prev_u[i]);
        S->part_u[i] = s1*S->part_u[i] - s*S->prev_u[i];
        S->prev_u[i]=u;
      }
    }
    S->load_scal=this->step_scal * FLOAT_SOLV(this->load_step);
    S->Init(E,Y);// FIXME repeated twice here
    for(uint i=0;i<Y->udof_magn.size();i++){
      //printf("GLOBAL MAX BC[%u]: %f\n",i,bcmax[i]);
      if(Y->udof_magn[i] > bcmax[i]){
#pragma omp atomic write
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
  //NOTE Nonlinear physics needs unscaled part_u.
#if 1
  //NOTE part_u is unscaled in this solver.
  for(uint i=0;i<bcmax.size();i++){ if(bcmax[i]<=0.0){ bcmax[i]=1.0; } }
#else
  // Set all DOF scales to unity.
  for(uint i=0;i<bcmax.size();i++){ bcmax[i]=1.0; }
#endif
#if VERB_MAX>2
    if(verbosity>2){
      printf("    DOF Scales:");
      for(uint i=0;i<bcmax.size();i++){ printf(" %g",bcmax[i]); }
      printf("\n");
    }
#endif
}
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    S->solv_cond=this->solv_cond;
    for(uint i=0;i<Y->udof_magn.size();i++){
//#pragma omp atomic read
      Y->udof_magn[i] = bcmax[i];
      S->glob_bmax[i] = this->glob_bmax[i];
    }
    //S->Init(E,Y);
    for(uint i=0; i<S->udof_n; i++){ S->part_d[i]=0.0; };
    S->Precond( E,Y );
  }
  time_reset( my_prec_count, start );
  //----------------------------- Sync part_d
#pragma omp single
{  this->halo_val=0.0; }
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH d=uint(Y->node_d);
    if(this->solv_cond == Solv::COND_NONE){
      for(INT_MESH i=0; i<E->halo_node_n; i++){
        for( uint j=0; j<d; j++){
#pragma omp atomic write
          this->halo_val[d*E->node_haid[i]+j] = S->part_d[d*i +j]; } }
    }else{
      for(INT_MESH i=0; i<E->halo_node_n; i++){
        for( uint j=0; j<d; j++){
#pragma omp atomic update
          this->halo_val[d*E->node_haid[i]+j]+= S->part_d[d*i +j]; } }
    }
  }
  time_reset( my_gat1_count, start );
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->part_d[d*i +j] = this->halo_val[f+j]; }
    }
  }
  time_reset( my_scat_count, start );// part_d sync -----
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){// ------------- Invert part_d
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH sysn=S->udof_n;
    for(uint i=0;i<sysn;i++){ S->part_d[i] = FLOAT_SOLV(1.0) / S->part_d[i]; }
    S->Init( E,Y );// Zeros boundary conditions
    //S->BCS( E,Y );
    //S->BC0( E,Y );
  }
  time_reset( my_solv_count, start );//FIXME wtf?
  time_reset( my_gat0_count, start );//FIXME wtf?
#pragma omp single
{   this->halo_val = 0.0; }// serial halo_vals zero
#ifdef HALO_SUM_SER
#pragma omp single
#else
#pragma omp for schedule(OMP_SCHEDULE)
#endif
  for(int part_i=part_0; part_i<part_o; part_i++){// --------------- sync part_f
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH sysn=S->udof_n;
    const INT_MESH d=uint(Y->node_d);
  //FIXME Put the element response calcs into the halo ops properly
    for(INT_MESH i=1; i<sysn; i++){ S->part_f[i]=0.0; }
    Y->ElemLinear( E,0,E->elem_n,S->part_f,S->part_u );
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic update
        this->halo_val[f+j] += S->part_f[d*i +j]; }
    }
  }// End halo_vals
  time_reset( my_gat1_count, start );
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->part_f[d*i +j] = this->halo_val[f+j]; }
    }
  time_reset( my_scat_count, start );// ----------------------- Done part_f sync
#if 0
#pragma omp critical(init)
{ S->Init(); }// Initial residual //FIXME Why is this serialized?
  glob_r2a += S->loca_res2;
#else
//FIXME Took this out of S->Init()
  }
#pragma omp for schedule(OMP_SCHEDULE) reduction(+:glob_r2a)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    const uint sysn=S->udof_n;// loca_res2 is a member variable.
    const uint sumi0=S->halo_loca_0;
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(uint i=0; i<sysn; i++){
      S->prev_r[i] = 0.0;// S->part_b[i] = 0.0;
      //S->part_r[i] = S->part_b[i] - S->part_f[i];
      S->part_b[i]-= S->part_f[i] * S->part_1[i];
      S->part_r[i] = S->part_b[i] - S->part_f[i];
      // Initial search (p) is preconditioned grad descent of (r)
      S->part_p[i] = S->part_r[i] * S->part_d[i];
#if 0
      if(S->part_d[i]==0.0){S->part_0[i]=0.0;}else{S->part_0[i]=1.0;}
#endif
    }
    //FLOAT_SOLV R2=0.0;
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd reduction(+:glob_r2a)
#endif
    for(uint i=sumi0; i<sysn; i++){
      glob_r2a += S->part_r[i]*S->part_r[i] * S->part_0[i]; }//FIXED div out precond
#if 0
    S->loca_res2=R2;
    S->loca_rto2 = S->loca_rtol*S->loca_rtol *S->loca_res2;
    glob_r2a += R2;// global initial residual reduction
#endif
#endif
  }
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
    S->loca_rto2 = S->loca_rtol*S->loca_rtol *glob_r2a;//FIXME wtf?
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
  this->glob_rto2 = glob_to2;
  return 0;
}//======================================================== End HaloNCG::Init()
int HaloNCG::Iter(){// printf("*** HaloNCG::Iter() ***\n");
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  FLOAT_SOLV glob_sum1=0.0, glob_sum2=0.0, glob_sum3=0.0, glob_sum4=0.0,
    glob_sum5=0.0;
  FLOAT_SOLV halo_vals[this->halo_val.size()];// Put this on the stack.
  //FIXME don't need M->halo_val member variable now.
#ifdef HALO_SUM_CHK
  FLOAT_SOLV halo_sers[this->halo_val.size()];
  FLOAT_SOLV halo_min= 9e9, halo_max=-9e9;//, halo_avg=0.0;
#endif
#pragma omp parallel num_threads(comp_n)
{// iter parallel region
  // HaloNCG::priv_part is a global threadprivate variable
  int part_0=0; if(std::get<0>( priv_part[0] )==NULL){ part_0=1; };
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
  //------------------------------------------- Compute and sync part_g = K( q )
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    const auto sysn = S->udof_n;
    time_start( solv_start );
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(uint i=0;i<sysn;i++){// Compute part_q = part_u + part_p
      S->part_q[i] = S->part_u[i] + S->part_p[i]; }//                   (1*N FLOP)
    time_accum( my_solv_count, solv_start );
    time_start( phys_start );
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(uint i=0;i<sysn;i++){ S->part_g[i]=0.0; }
    Y->ElemNonlinear( E,0,E->halo_elem_n, S->part_g, S->part_q, S->part_u, false );
    time_accum( my_phys_count, phys_start );
    time_start( gath_start );
    const INT_MESH hnn=E->halo_node_n,hrn=E->halo_remo_n;
    for(INT_MESH i=hrn; i<hnn; i++){//NOTE memcpy apparently not critical
      std::memcpy(& halo_vals[Dn* E->node_haid[i]], & S->part_g[Dn* i],
        Dn*sizeof(FLOAT_PHYS) ); }
    time_accum( my_gat0_count, gath_start );
  }
#ifdef HALO_SUM_CHK
  std::memcpy(& halo_sers[0],& halo_vals[0],
              this->halo_val.size()*sizeof(FLOAT_PHYS) );
#endif
#ifdef HALO_SUM_SER
#pragma omp single
#else
#pragma omp for schedule(OMP_SCHEDULE)
#endif
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    time_start( gath_start );
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      const auto f = Dn* E->node_haid[i];
      for( uint j=0; j<Dn; j++){
#pragma omp atomic update
       halo_vals[f+j]+= S->part_g[Dn* i+j]; }
    }
    time_accum( my_gat1_count, gath_start );
  }// End halo_vals sum; now scatter back to elems
#ifdef HALO_SUM_CHK
#pragma omp single
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      const auto f = Dn* E->node_haid[i];
      for( uint j=0; j<Dn; j++){
#pragma omp atomic update
       halo_sers[f+j]+= S->part_g[Dn* i+j]; }
    }
  }// End serial halo_vals sum;
#pragma omp single
{// Report discrepancies
  INT_MESH s = this->halo_val.size();
  printf("part_g halo[%u] errors: ",s);
  for(INT_MESH i=0; i<s; i++){
    FLOAT_SOLV e = halo_vals[i]-halo_sers[i];
    if( e < halo_min ){ halo_min=e; }
    if( e > halo_max ){ halo_max=e; }
    //if(!isnan(e)){ halo_avg += e; }
  }
  //halo_avg = halo_avg / double( s );
  printf("%+6e < %+6e",halo_min,halo_max);
  printf("\n");
}
#endif
#pragma omp for schedule(OMP_SCHEDULE) reduction(+:glob_sum1,glob_sum2)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    time_start( scat_start );
    const INT_MESH hnn=E->halo_node_n,hl0=S->halo_loca_0,sysn=S->udof_n;
    for(INT_MESH i=0; i<hnn; i++){//NOTE appears not to be critical
      std::memcpy(& S->part_g[Dn* i],& halo_vals[Dn* E->node_haid[i]],
        Dn*sizeof(FLOAT_PHYS) ); }
    time_accum( my_scat_count, scat_start );
    time_start( phys_start );
    Y->ElemNonlinear( E,E->halo_elem_n,E->elem_n, S->part_g, S->part_q,
                      S->part_u, false );
    time_accum( my_phys_count, phys_start );
    //--------------------------------------------- Done compute and sync part_g
    time_start( solv_start );
    //----------------------------------------------------- Actual method alpha
#if 0
    for(INT_MESH i=0; i<sysn; i++){
      S->part_f[i] = S->part_f[i] * S->part_0[i];
      S->part_g[i] = S->part_g[i] * S->part_0[i];// part_g is only used once
    }
#endif
    // Actual method is equivalent to secant for constant RHS
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd reduction(+:glob_sum1,glob_sum2)
#endif
    for(INT_MESH i=hl0; i<sysn; i++){
      glob_sum1+= S->part_p[i] * S->part_r[i];// alpha numerator
      glob_sum2+= S->part_p[i] * S->part_0[i]*(S->part_g[i] - S->part_f[i]);//denom
    }//                                                              (6*N FLOP)
    time_accum( my_solv_count, solv_start );
  }
  const FLOAT_SOLV alpha = glob_sum1 / glob_sum2;//                    (1 FLOP)
  //printf("ALPHA:%+9.2e\n",alpha);
  //----------------------------------------------------- Update solution part_u
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    const auto sysn = S->udof_n;
    time_start( solv_start );
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(INT_MESH i=0; i<sysn; i++){
      S->part_f[i]=0.0;
      S->part_u[i]+=alpha * S->part_p[i]; }//* S->part_d[i]; }//      (2*N FLOP)
    time_accum( my_solv_count, solv_start );//FIXED Merge with next loop
#if 0
  }
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    const auto sysn = S->udof_n;
#endif
    //------------------------------------------------- Compute and sync forces
    const INT_MESH Dn=uint(Y->node_d);
    time_start( phys_start );
    Y->ElemLinear( E,0,E->halo_elem_n, S->part_f, S->part_u );
    time_accum( my_phys_count, phys_start );
    time_start( gath_start );
    const INT_MESH hnn=E->halo_node_n,hrn=E->halo_remo_n;
    for(INT_MESH i=hrn; i<hnn; i++){//NOTE memcpy apparently not critical
      std::memcpy(& halo_vals[Dn* E->node_haid[i]],& S->part_f[Dn* i],
        Dn*sizeof(FLOAT_PHYS) ); }
    time_accum( my_gat0_count, gath_start );
  }
#ifdef HALO_SUM_SER
#pragma omp single
#else
#pragma omp for schedule(OMP_SCHEDULE)
#endif
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    time_start( gath_start );
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      const auto f = Dn* E->node_haid[i];
      for( uint j=0; j<Dn; j++){
#pragma omp atomic update
        halo_vals[f+j]+= S->part_f[Dn* i+j];
      }
    }
    time_accum( my_gat1_count, gath_start );
  }// End halo_vals sum; now scatter back to elems
#pragma omp for schedule(OMP_SCHEDULE) reduction(+:glob_sum3,glob_sum4,glob_sum5)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    time_start( scat_start );
    const INT_MESH hnn=E->halo_node_n;
    for(INT_MESH i=0; i<hnn; i++){//NOTE appears not to be critical
      std::memcpy(& S->part_f[Dn* i],& halo_vals[Dn* E->node_haid[i]],
        Dn*sizeof(FLOAT_PHYS) ); }
    time_accum( my_scat_count, scat_start );
    time_start( phys_start );
    Y->ElemLinear( E,E->halo_elem_n,E->elem_n, S->part_f, S->part_u );
    time_accum( my_phys_count, phys_start );
    //--------------------------------------------- Done compute and sync part_f
    //-------------------------------------- Calculate negative residuals part_r
    time_start( solv_start );
    const INT_MESH hl0=S->halo_loca_0,sysn=S->udof_n;
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(INT_MESH i=0; i<sysn; i++){
      S->prev_r[i] = S->part_r[i];
#if 0
      S->part_f[i]*= S->part_0[i];// better to apply where used
#endif
      //S->part_r[i] = S->part_b[i] - S->part_f[i]; }
      S->part_r[i] = S->part_0[i] *(S->part_b[i] - S->part_f[i]); }
      //S->part_r[i] = S->part_b[i] - S->part_0[i] - S->part_f[i]; }
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd reduction(+:glob_sum3,glob_sum4,glob_sum5)
#endif
    for(INT_MESH i=hl0; i<sysn; i++){//------------------ Reduce residual norms
#if 0
      // FletcherReeves
      glob_sum3 += S->part_r[i] * S->part_d[i] * S->part_r[i];
      glob_sum4 += S->prev_r[i] * S->part_d[i] * S->prev_r[i];
      glob_sum5 += S->part_r[i] * S->part_r[i];
#else
      // PolakRibiere (SM default)
      glob_sum3 += S->part_r[i] * S->part_d[i] *(S->part_r[i] - S->prev_r[i]);
      glob_sum4 += S->prev_r[i] * S->part_d[i] * S->prev_r[i];
      glob_sum5 += S->part_r[i] * S->part_r[i];
#endif
    }//                                                              (9*N FLOP)
    time_accum( my_solv_count, solv_start );
  }
  //----------------------------------------------------- Compute beta (1 FLOP)
#if 1
  // I think this is the SM restart criterion.
  // The value of glob_sum3 / glob_sum4 measures non-orthogonality.
  FLOAT_SOLV beta = glob_sum3 / glob_sum4;
  beta *= FLOAT_SOLV(beta>0.1);
#else
  const FLOAT_SOLV beta = std::max(0.0, glob_sum3 / glob_sum4 );
  //NOTE the max provides a search direction reset automatically.
#endif
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){// Update search direction
    std::tie(E,Y,S)=priv_part[part_i];
    const INT_MESH sysn=S->udof_n;
    time_start( solv_start );
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(INT_MESH i=0; i<sysn; i++){
      S->part_p[i] = S->part_r[i] * S->part_d[i] + beta * S->part_p[i]; }
    time_accum( my_solv_count, solv_start );
  }//                                                                (3*N FLOP)
#if VERB_MAX>1
  iter_done  = std::chrono::high_resolution_clock::now();
  //auto solv_time  = std::chrono::duration_cast<std::chrono::nanoseconds>
   // (iter_done - solv_start);
  auto iter_time  = std::chrono::duration_cast<std::chrono::nanoseconds>
    (iter_done - iter_start);// printf("%i ",iter);
#pragma omp critical(time)
{
  this->time_secs[0]+=float(my_phys_count)*1e-9;
  this->time_secs[1]+=float(my_gat0_count)*1e-9;
  this->time_secs[2]+=float(my_gat1_count)*1e-9;
  this->time_secs[3]+=float(my_scat_count)*1e-9;
  this->time_secs[4]+=float(my_solv_count)*1e-9;
  //this->time_secs[4]+=float(my_solv_count+solv_time.count())*1e-9;
  //this->time_secs[4]+=float(solv_time.count())*1e-9;
  this->time_secs[5]+=float(iter_time.count())*1e-9;
}
#endif
}// end iter parallel region
  this->glob_res2 = glob_sum3;
  this->glob_chk2 = glob_sum5;
  return 0;
}
