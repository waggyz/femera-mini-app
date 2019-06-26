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
std::vector<Mesh::part> HaloNCG::P;
//std::vector<Mesh::part> HaloNCG::Ptoto;
//
int NCG::BC (Mesh* ){return 1;}
int NCG::RHS(Mesh* ){return 1;}
int NCG::Iter(){return 1;}
int NCG::Solve( Elem*, Phys* ){return 1;}
//
int NCG::RHS(Elem* E, Phys* Y ){// printf("*** NCG::RHS(E,Y) ***\n");
  const uint Dn=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->rhs_vals ){ std::tie(n,f,v)=t;
    this->sys_b[Dn* n+uint(f)]+= v;//FIXME apply preconditioner sys_d?
  }
  return(0);
}
int NCG::BCS(Elem* E, Phys* Y ){// printf("*** NCG::BCS(E,Y) ***\n");
  uint Dn=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    //printf("FIX ID %i, DOF %i, val %+9.2e\n",i,E->bcs_vals[i].first,E->bcs_vals[i].second);
    this->sys_u[Dn* n+uint(f)] = v;
    if(std::abs(v) > Y->udof_magn[f]){ Y->udof_magn[f] = std::abs(v); }
    //FIXME v * sys_d[]???
  }
  return(0);
}
int NCG::BC0(Elem* E, Phys* Y ){// printf("*** NCG::BC0(E,Y) ***\n");
  uint Dn=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    this->sys_d[Dn* n+uint(f)]=0.0;
    this->sys_f[Dn* n+uint(f)]=0.0;
  }
  for(auto t : E->bc0_nf   ){ std::tie(n,f)=t;
    this->sys_d[Dn* n+uint(f)]=0.0;
    #if VERB_MAX>10
    printf("BC0: [%i]:0\n",E->bc0_nf[i]);
    #endif
  }
  return(0);
}
int NCG::Setup( Elem* E, Phys* Y ){// printf("*** NCG::Setup(E,Y) ***\n");
  //this->meth_name="preconditioned conjugate gradient";
  this->halo_loca_0 = E->halo_remo_n * Y->node_d;
  this->RHS( E,Y );
  this->BCS( E,Y );// Sync max Y->udof_magn before Precond()
  E->do_halo=true ; Y->ElemLinear( E,this->sys_f,this->sys_u );
  E->do_halo=false; Y->ElemLinear( E,this->sys_f,this->sys_u );
  return(0);
}
int NCG::Init( Elem* E, Phys* Y ){// printf("*** NCG::Init(E,Y) ***\n");
  this->BC0( E,Y );
  return 0;
}
int NCG::Init(){// printf("*** NCG::Init() ***\n");
  const uint sysn=this->udof_n;// loca_res2 is a member variable.
  const uint sumi0=this->halo_loca_0;
  FLOAT_SOLV R2=0.0;
#pragma omp simd
  for(uint i=0; i<sysn; i++){
    //this->sys_b[i] -= this->sys_f[i]*this->sys_d[i];
    this->sys_b[i]*= sys_d[i];
    this->sys_f[i]*= sys_d[i];
    this->sys_p[i] = this->sys_b[i] - this->sys_f[i];//*this->sys_d[i];//FIXME sign?
    this->sys_r[i] = this->sys_p[i];//FIXME initial search (p) is grad. desc. (r)
  }
#pragma omp simd reduction(+:R2)
  for(uint i=sumi0; i<sysn; i++){
    R2 += this->sys_p[i]*this->sys_p[i]; }
  this->loca_res2=R2;
  this->loca_rto2 = this->loca_rtol*loca_rtol *loca_res2;//FIXME Move this somewhere.
  return(0);
}
int HaloNCG::Init(){// printf("*** HaloNCG::Init() ***\n");
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  INT_MESH halo_n=0;
  // Local copies for atomic ops and reduction
  //FLOAT_SOLV glob_r2a = this->glob_res2, glob_to2 = this->glob_rto2;
  FLOAT_SOLV glob_r2a = 0.0, glob_to2 = this->glob_rto2;
  Phys::vals bcmax={0.0,0.0,0.0,0.0};
#pragma omp parallel num_threads(comp_n)
{// parallel init region
  long int my_scat_count=0, my_prec_count=0,
    my_gat0_count=0,my_gat1_count=0, my_gmap_count=0, my_solv_count=0;
  auto start = std::chrono::high_resolution_clock::now();
  // Make thread-local copies of mesh_part into threadprivate P.
  P.resize(this->mesh_part.size());
  std::copy(this->mesh_part.begin(), this->mesh_part.end(), P.begin());
  int part_0=0; if(std::get<0>( P[0] )==NULL){ part_0=1; }
  const int part_n = int(P.size())-part_0;
  const int part_o = part_n+part_0;
  // Sync max Y->udof_magn
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    for(uint i=0;i<Y->udof_magn.size();i++){
      //printf("GLOBAL MAX BC[%u]: %f\n",i,bcmax[i]);
      if(Y->udof_magn[i] > bcmax[i]){
//#pragma omp atomic write
        bcmax[i]=Y->udof_magn[i];
      }
    }
  }
#pragma omp master
{
  auto m=bcmax[0];
  for(uint i=1;i<3;i++){ if(bcmax[0] > m){ m=bcmax[i]; } }
  for(uint i=0;i<3;i++){ bcmax[i]=m; }
  for(uint i=0;i<bcmax.size();i++){ if(bcmax[i]<=0.0){ bcmax[i]=1.0; } }
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
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    S->solv_cond=this->solv_cond;
    for(uint i=0;i<Y->udof_magn.size();i++){
//#pragma omp atomic read
      Y->udof_magn[i] = bcmax[i];
    }
    S->Precond( E,Y );
  }
  // Sync sys_d [this inits M->halo_map and E->node_haid]//FIXME separate
  time_reset( my_prec_count, start );//----------- Init halo map and sync sys_d
#pragma omp for schedule(OMP_SCHEDULE)
    for(int part_i=part_0; part_i<part_o; part_i++){
      Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
      if(E->node_haid.size()==0){ E->node_haid.resize(E->halo_node_n); }
    }
#pragma omp for schedule(OMP_SCHEDULE)
    for(int part_i=part_0; part_i<part_o; part_i++){
      Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
      //if(E->node_haid.size()==0){ E->node_haid.resize(E->halo_node_n); };
      const INT_MESH d=uint(Y->node_d);
      for(INT_MESH i=0; i<E->halo_node_n; i++){
#pragma omp critical(halomap)
{//FIXME critical section here?
      INT_MESH g=E->node_glid[i];
      if(this->halo_map.count(g)==0){// Initialize halo_map
        this->halo_map[g]=halo_n;
        E->node_haid[i]=halo_n;
        for( uint j=0; j<d; j++){
          this->halo_val[d*E->node_haid[i]+j]  = S->sys_d[d*i +j]; }
        halo_n++;
      }else{// Add in the rest.
        E->node_haid[i]=this->halo_map[g];
        if(this->solv_cond == Solv::COND_NONE){
          for( uint j=0; j<d; j++){
            this->halo_val[d*E->node_haid[i]+j] = S->sys_d[d*i +j]; } }
        else{
          for( uint j=0; j<d; j++){
            this->halo_val[d*E->node_haid[i]+j]+= S->sys_d[d*i +j]; } }
      }
}
    }// End halo_map loop
    }// End sys_d gather parts
    time_reset( my_gmap_count, start );
#pragma omp for schedule(OMP_SCHEDULE)
    for(int part_i=part_0; part_i<part_o; part_i++){
      Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
      const INT_MESH d=uint(Y->node_d);
      for(INT_MESH i=0; i<E->halo_node_n; i++){
        auto f = d* E->node_haid[i];
        for( uint j=0; j<d; j++){
#pragma omp atomic read
          S->sys_d[d*i +j] = this->halo_val[f+j]; }
      }
    }// end sys_d scatter parts
  time_reset( my_scat_count, start );// Done halo_map init and sys_d sync -----
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){// ------------- Invert sys_d
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH sysn=S->udof_n;
    for(uint i=0;i<sysn;i++){ S->sys_d[i] = FLOAT_SOLV(1.0) / S->sys_d[i]; }
//#pragma omp critical
//{
    S->Init( E,Y );// Zeros boundary conditions
//}
  }
  time_reset( my_solv_count, start );//FIXME wtf?
  time_reset( my_gat0_count, start );//FIXME wtf?
#pragma omp single nowait
{   this->halo_val = 0.0; }// serial halo_vals zero
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){// --------------- sync sys_f
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic update
        this->halo_val[f+j] += S->sys_f[d*i +j]; }
    }
  }// End halo_vals
  time_reset( my_gat1_count, start );
#pragma omp for schedule(OMP_SCHEDULE) reduction(+:glob_r2a)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->sys_f[d*i +j] = this->halo_val[f+j]; }
    }
  time_reset( my_scat_count, start );// ----------------------- Done sys_f sync
#pragma omp critical(init)
{ S->Init(); }// Initial residual //FIXME Why is this serialized?
  glob_r2a += S->loca_res2;// global initial residual reduction
  }
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    S->loca_rto2 = S->loca_rtol*S->loca_rtol *glob_r2a;//FIXME wtf?
#pragma omp atomic write
    glob_to2 = S->loca_rto2;// Pass the relative tolerance out.
  }
  time_reset( my_solv_count, start );
#if VERB_MAX>1
#pragma omp critical(time)
{
  this->time_secs[0]+=float(my_prec_count)*1e-9;
  this->time_secs[1]+=float(my_gmap_count)*1e-9;
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
int HaloNCG::Iter(){// printf("*** HaloNCG::Iter() ***\n");
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  FLOAT_SOLV glob_sum1=0.0, glob_sum2=0.0, glob_sum3=0.0;
  FLOAT_SOLV glob_r2a = this->glob_res2;
#pragma omp parallel num_threads(comp_n)
{// iter parallel region
// not needed anymore,since P is threadprivate
//  std::vector<part> P; P.resize(this->mesh_part.size());
//  std::copy(this->mesh_part.begin(), this->mesh_part.end(), P.begin());
  int part_0=0; if(std::get<0>( P[0] )==NULL){ part_0=1; };
  const int part_n = int(P.size())-part_0;
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
  //------------------------------------------- Compute and sync sys_g = K( q )
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=P[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    const auto sysn = S->udof_n;
    time_start( solv_start );
#pragma omp simd
    for(uint i=0;i<sysn;i++){// Compute sys_q = sys_u + sys_p
      S->sys_q[i] = S->sys_u[i] + S->sys_p[i]; }
    time_accum( my_solv_count, solv_start );
    time_start( phys_start );
#pragma omp simd
    for(uint i=0;i<sysn;i++){ S->sys_g[i]=0.0; }
    E->do_halo=true; Y->ElemLinear( E, S->sys_g, S->sys_q );
    time_accum( my_phys_count, phys_start );
    time_start( gath_start );
    const INT_MESH hnn=E->halo_node_n,hrn=E->halo_remo_n;
    for(INT_MESH i=hrn; i<hnn; i++){//NOTE memcpy apparently not critical
      std::memcpy(
        & this->halo_val[Dn* E->node_haid[i]],
        & S->sys_g[Dn* i],
        Dn*sizeof(FLOAT_PHYS) );
    }
    time_accum( my_gat0_count, gath_start );
  }
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=P[part_i];
    time_start( gath_start );
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      const auto f = Dn* E->node_haid[i];
      for( uint j=0; j<Dn; j++){
#pragma omp atomic update
        this->halo_val[f+j]+= S->sys_g[Dn* i+j]; }
    }
    time_accum( my_gat1_count, gath_start );
  }// End halo_vals sum; now scatter back to elems
#pragma omp for schedule(OMP_SCHEDULE) reduction(+:glob_sum1,glob_sum2)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=P[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    time_start( scat_start );
    const INT_MESH hnn=E->halo_node_n,hl0=S->halo_loca_0,sysn=S->udof_n;
    for(INT_MESH i=0; i<hnn; i++){//NOTE appears not to be critical
      std::memcpy(
        & S->sys_g[Dn* i],
        & this->halo_val[Dn* E->node_haid[i]],
        Dn*sizeof(FLOAT_PHYS) );
    }
    time_accum( my_scat_count, scat_start );
    time_start( phys_start );
    E->do_halo=false; Y->ElemLinear( E, S->sys_g, S->sys_q );
    time_accum( my_phys_count, phys_start );
    //--------------------------------------------- Done compute and sync sys_g
//  }
//#pragma omp for schedule(OMP_SCHEDULE) reduction(+:glob_sum1,glob_sum2)
//  for(int part_i=part_0; part_i<part_o; part_i++){
//    std::tie(E,Y,S)=P[part_i];
//    const auto sysn = S->udof_n,hl0=S->halo_loca_0;
    time_start( solv_start );
#pragma omp simd
    for(INT_MESH i=0; i<sysn; i++){
      S->sys_g[i] *= S->sys_d[i];
      //S->sys_g[i] = S->sys_b[i] - S->sys_g[i] * S->sys_d[i];
    }
#pragma omp simd reduction(+:glob_sum1,glob_sum2)
    for(INT_MESH i=hl0; i<sysn; i++){
      glob_sum1 += S->sys_p[i] * S->sys_r[i];// alpha numerator
      glob_sum2 += S->sys_p[i] * ( S->sys_g[i] - S->sys_f[i] );// alpha denominator
    }
    time_accum( my_solv_count, solv_start );//FIXME?
  }
  //----------------------------------------------------------- Calculate alpha
  const FLOAT_SOLV alpha = glob_sum1 / glob_sum2;// 1 FLOP
  //printf("ALPHA:%+9.2e\n",alpha);
  //----------------------------------------------------- Update solution sys_u
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){//FIXME Merge with next loop
    std::tie(E,Y,S)=P[part_i];
    const auto sysn = S->udof_n;
    time_start( solv_start );
#pragma omp simd
    for(INT_MESH i=0; i<sysn; i++){
      S->sys_u[i] += alpha * S->sys_p[i]; }
    time_accum( my_solv_count, solv_start );
  }
  //--------------------------------------------- Compute and sync forces sys_f
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=P[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    time_start( phys_start );
    const auto sysn = S->udof_n;
#pragma omp simd
    for(uint i=0;i<sysn;i++){ S->sys_f[i]=0.0; }
    E->do_halo=true; Y->ElemLinear( E, S->sys_f, S->sys_u );
    time_accum( my_phys_count, phys_start );
    time_start( gath_start );
    const INT_MESH hnn=E->halo_node_n,hrn=E->halo_remo_n;
    for(INT_MESH i=hrn; i<hnn; i++){//NOTE memcpy apparently not critical
      std::memcpy(
        & this->halo_val[Dn* E->node_haid[i]],
        & S->sys_f[Dn* i],
        Dn*sizeof(FLOAT_PHYS) );
    }
    time_accum( my_gat0_count, gath_start );
  }
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=P[part_i];
    time_start( gath_start );
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      const auto f = Dn* E->node_haid[i];
      for( uint j=0; j<Dn; j++){
#pragma omp atomic update
        this->halo_val[f+j]+= S->sys_f[Dn* i+j]; }
    }
    time_accum( my_gat1_count, gath_start );
  }// End halo_vals sum; now scatter back to elems
#pragma omp for schedule(OMP_SCHEDULE) reduction(+:glob_sum3)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=P[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    time_start( scat_start );
    const INT_MESH hnn=E->halo_node_n;//,hl0=S->halo_loca_0,sysn=S->udof_n;
    for(INT_MESH i=0; i<hnn; i++){//NOTE appears not to be critical
      std::memcpy(
        & S->sys_f[Dn* i],
        & this->halo_val[Dn* E->node_haid[i]],
        Dn*sizeof(FLOAT_PHYS) );
    }
    time_accum( my_scat_count, scat_start );
    time_start( phys_start );
    E->do_halo=false; Y->ElemLinear( E, S->sys_f, S->sys_u );
    time_accum( my_phys_count, phys_start );
    //--------------------------------------------- Done compute and sync sys_f
    //-------------------------------------- Calculate negative residuals sys_r
    time_start( solv_start );
    const INT_MESH hl0=S->halo_loca_0,sysn=S->udof_n;
#pragma omp simd
    for(INT_MESH i=0; i<sysn; i++){
      S->sys_f[i] *= S->sys_d[i];
      S->sys_r[i]  = S->sys_b[i] - S->sys_f[i]; }
#pragma omp simd reduction(+:glob_sum3)
    for(INT_MESH i=hl0; i<sysn; i++){// Reduce residual norm
      glob_sum3 += S->sys_r[i] * S->sys_r[i]; }
    time_accum( my_solv_count, solv_start );
  }
  //-------------------------------------------------------------- Compute beta
  const FLOAT_SOLV beta = glob_sum3 / glob_r2a;// this R2 / last R2
  //--------------------------------------------- Update search direction sys_p
#pragma omp for schedule(OMP_SCHEDULE)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=P[part_i];
    const INT_MESH sysn=S->udof_n;
    time_start( solv_start );
#pragma omp simd
    for(INT_MESH i=0; i<sysn; i++){
      S->sys_p[i] = S->sys_r[i] + beta * S->sys_p[i];
    }
    time_accum( my_solv_count, solv_start );
  }
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
  this->glob_chk2 = glob_sum3;
  return 0;
}