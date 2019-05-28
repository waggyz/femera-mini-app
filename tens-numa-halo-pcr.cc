#include <utility>//std::pair
#include <vector>
#include <set>// This is ordered
#include <algorithm>    // std::copy
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

int PCR::BC (Mesh* ){return 0;};
int PCR::RHS(Mesh* ){return 0;};
//
int PCR::RHS(Elem* E, Phys* Y ){
  uint d=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->rhs_vals ){ std::tie(n,f,v)=t;
    this->sys_r[d* n+uint(f)]+= v;
  };
  return(0);
};
int PCR::BCS(Elem* E, Phys* Y ){
  uint d=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    this->sys_u[d* n+uint(f)] = v;
  };
  return(0);
};
int PCR::BC0(Elem* E, Phys* Y ){
  uint d=uint(Y->node_d);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    this->sys_d[d* n+uint(f)]=0.0;
    this->sys_f[d* n+uint(f)]=0.0;
  };
  for(auto t : E->bc0_nf   ){ std::tie(n,f)=t;
    this->sys_d[d* n+uint(f)]=0.0;
    #if VERB_MAX>10
    printf("BC0: [%i]:0\n",E->bc0_nf[i]);
    #endif
  };
  return(0);
};
int PCR::Setup( Elem* E, Phys* Y ){
  //this->meth_name="preconditioned conjugate residual";
  this->halo_loca_0 = E->halo_remo_n * Y->node_d;
  //this->udof_n = E->node_n * Y->node_d;
  //this->udof_flop = 14;//*elem_n
  //this->udof_band = 17*sizeof(FLOAT_SOLV);//*udof_n + 2
  //this->Setup();
  //sys_u.resize(udof_n,0.0);// Initial Solution Guess
  //sys_f.resize(udof_n,0.0);// CR response vector f=Ar
  //if(sys_d.size()<udof_n){ sys_d.resize(udof_n,1.0); };
  // Default Diagonal Preconditioner
  //sys_r.resize(udof_n,0.0);// Residuals
  //sys_p.resize(udof_n,0.0);// CR working vector
  //sys_g.resize(udof_n,0.0);// CR response vector g=Ap
  //
  //sys_d=0.0; Y->ElemJacobi( E, this->sys_d );
  //this->Precond( E,Y );//FIXME Precond doesn't rotate yet
  //FIXME Report returned int: negative values corrected.
  //NOTE sync sys_d before inverting it.
  //
  this->RHS( E,Y );// Sets sys_r to RHS
  this->BCS( E,Y );
  E->do_halo=true;
  Y->ElemLinear( E,this->sys_f,this->sys_u );
  E->do_halo=false;
  Y->ElemLinear( E,this->sys_f,this->sys_u );
  return(0);
};
int PCR::Init( Elem* E, Phys* Y ){
  this->sys_d = FLOAT_SOLV(1.0) / this->sys_d;
  this->BC0( E,Y );
  this->sys_r -= this->sys_f;
  sys_r *= sys_d;
  sys_f=0.0;
  E->do_halo=true;
  Y->ElemLinear( E,this->sys_f,this->sys_r );
  E->do_halo=false;
  Y->ElemLinear( E,this->sys_f,this->sys_r );
  return 0;
};
int PCR::Init(){
  const uint n=sys_u.size();
  const uint sumi0=this->halo_loca_0;// *this->node_d;
  sys_g = sys_f;
  sys_p = sys_r;
  FLOAT_SOLV s=0.0;
  for(uint i=sumi0; i<n; i++){// 2 FLOP/DOF, 2 read = 2/DOF
    s += sys_r[i] * sys_f[i]; };
  this->loca_res2=s;
  //this->to2 = this->tol*tol *loca_res2;//FIXED Move this somewhere.
  this->loca_rto2 = loca_rtol*loca_rtol *loca_res2;//FIXME Move this somewhere.
  //
  //if( loca_res2 < to2 ){ return(SOLV_CNVG_PTOL); }
  //else{ return(0); };
  return 0;
};
//int PCR::Iter(Elem* E,Phys* Y){// 2 FLOP + 14 FLOP/DOF
int PCR::Iter(){// 2 FLOP + 14 FLOP/DOF, 17 mem/DOF
  //NOTE Compute initial sys_f=sys_g=[k][r] before iterating with this?
  //FIXME Compute current sys_f before iterating with this?
  const uint n=sys_u.size();
  const uint sumi0=this->halo_loca_0;// *this->node_d;//FIXME Magic number
  //const auto ra=this->loca_res2;// Make a local version of this member variable
  //--------------------------------------------
  //FLOAT_SOLV s=0.0;
  //for(uint i=sumi0; i<n; i++){
  //  s += sys_d[i] * sys_g[i] * sys_g[i];
  //};//printf("||r|| %9.2e\n",std::sqrt(s) );
  //const FLOAT_SOLV alpha  = loca_res2 / s;// 1 DIV FLOP
  //
  //for(uint i=0; i<n; i++){
  //  sys_u[i] += alpha * sys_p[i];
  //  sys_r[i] -= alpha * sys_d[i] * sys_g[i];
  //};
  //if( loca_res2 < to2 ){ return(SOLV_CNVG_PTOL); };
  //--------------------------------------------
  //this->sys_f = 0.0;
  //E->do_halo=true;
  //Y->ElemLinear( E,this->sys_f,this->sys_r );
  //E->do_halo=false;
  //Y->ElemLinear( E,this->sys_f,this->sys_r );
  //
  FLOAT_SOLV r2b=0.0;
  for(uint i=sumi0; i<n; i++){// 2 FLOP/DOF, 2 read =2/DOF
    r2b += sys_r[i] * sys_f[i];
  };
  const FLOAT_PHYS beta = r2b/loca_rto2;//  1 DIV FLOP
  this->loca_rto2=r2b;
  //
  for(uint i=0; i<n; i++){// 4 FLOP/DOF, 4 read +2 write =6/DOF
     sys_p[i] = sys_r[i] + beta * sys_p[i];
     sys_g[i] = sys_f[i] + beta * sys_g[i];
  };
  //NOTE Rearranged algorithm to do ElemLinear first, and converge check last
  FLOAT_SOLV s=0.0;
  for(uint i=sumi0; i<n; i++){// 3 FLOP/DOF, 2 read =2/DOF
    s += sys_d[i] * sys_g[i] * sys_g[i];
  };//printf("||r|| %9.2e\n",std::sqrt(s) );
  const FLOAT_SOLV alpha  = loca_rto2 / s;// 1 DIV FLOP
  //
  for(uint i=0; i<n; i++){// 5 FLOP/DOF, 5 read +2 write =7/DOF
    sys_u[i] += alpha * sys_p[i];//FIXME This may be one update too many
    sys_r[i] -= alpha * sys_d[i] * sys_g[i];
  };
  if( loca_rto2 < loca_rto2 ){ return(SOLV_CNVG_PTOL); }
  else{ return(0); };
};
int PCR::Solve( Elem* E, Phys* Y ){//FIXME Redo this
  //this->Setup( E,Y );
  this->Init( E,Y );
  if( !this->Init() ){//============ Solve ================
    // The following is needed here to rearrange 
    const uint n=sys_u.size();
    const uint sumi0=this->halo_loca_0;
    FLOAT_SOLV s=0.0;
    for(uint i=sumi0; i<n; i++){
      s += sys_d[i] * sys_g[i] * sys_g[i];
    };//printf("||r|| %9.2e\n",std::sqrt(s) );
    const FLOAT_SOLV alpha  = loca_res2 / s;
    for(uint i=0; i<n; i++){
      sys_u[i] += alpha * sys_p[i];
      sys_r[i] -= alpha * sys_d[i] * sys_g[i];
    };
    if( loca_res2 < loca_rto2 ){ return(SOLV_CNVG_PTOL); }
    //printf("SER INIT loca_res2:%9.2e /%9.2e\n",this->loca_res2,this->to2);
    printf("SER INIT ||R|:%9.2e /%9.2e\n",
      std::sqrt(this->loca_res2),std::sqrt(this->loca_rto2) );
    for(this->iter=0; this->iter < this->iter_max; this->iter++){
      this->sys_f=0.0;
      //FIXME HALO SYNC this->sys_f
      //if( this->Iter(E,Y) ){ break ;};// CR Iteration
      E->do_halo=true;
      Y->ElemLinear( E,this->sys_f,this->sys_r );
      E->do_halo=false;
      Y->ElemLinear( E,this->sys_f,this->sys_r );
      if( this->Iter() ){ break ;};// CR Iteration
#if VERB_MAX>1
      if(!((iter+1) % 100) ){
        printf("%6i ||R||%9.2e\n", iter+1, std::sqrt(loca_res2) ); };
#endif
    };// End iteration loop.
  };
  return 0;//FIXME
};
int HaloPCR::Init(){// Preconditioned Conjugate Residual
  int part_0=0;
  if(std::get<0>( this->mesh_part[0] )==NULL){ part_0=1; };
  const int part_n = int(this->mesh_part.size())-part_0;
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  //
  INT_MESH halo_n=0;
  // Local copies for atomic ops and reduction
  FLOAT_SOLV glob_r2a = 0.0, alpha=0.0;//this->glob_res2;
  FLOAT_SOLV glob_to2 = this->glob_rto2, glob_sum1=0.0;//glob_chk2=0.0, 
#pragma omp parallel num_threads(comp_n)
{// parallel init region
  long int my_scat_count=0, my_prec_count=0,
    my_gat0_count=0,my_gat1_count=0, my_gmap_count=0, my_solv_count=0;
  auto start = std::chrono::high_resolution_clock::now();
  // Sync sys_d [this inits halo_map]
  auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_prec_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
  //
  std::vector<Mesh::part> P;  P.resize(this->mesh_part.size());
  std::copy(this->mesh_part.begin(), this->mesh_part.end(), P.begin());
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<(part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    S->solv_cond=this->solv_cond;
    S->Precond( E,Y );
  };
#pragma omp for schedule(static)
for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
  Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
  if(E->node_haid.size()==0){ E->node_haid.resize(E->halo_node_n); };
  const INT_MESH d=uint(Y->node_d);
  for(INT_MESH i=0; i<E->halo_node_n; i++){
#pragma omp critical(halomap)
{//FIXME critical section here?
      INT_MESH g=E->node_glid[i];
      if(this->halo_map.count(g)==0){// Initialize halo_map
        this->halo_map[g]=halo_n;
        E->node_haid[i]=halo_n;
        for( uint j=0; j<d; j++){
          this->halo_val[d*E->node_haid[i]+j]  = S->sys_d[d*i +j]; };
        halo_n++;
      }else{// Add in the rest.
        E->node_haid[i]=this->halo_map[g];
        for( uint j=0; j<d; j++){
          this->halo_val[d*E->node_haid[i]+j] += S->sys_d[d*i +j]; };
      };
}
    };
  };// End sys_d gather
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_gmap_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->sys_d[d*i +j] = this->halo_val[f+j]; };
    };
  };// end sys_d scatter, now sync sys_f
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_scat_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp single
{   this->halo_val = 0.0; }// serial halo_vals zero
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_gat0_count +=dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic update
        this->halo_val[f+j] += S->sys_f[d*i +j]; };
    };
  };
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_gat1_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->sys_f[d*i +j] = this->halo_val[f+j]; };
    };
  };
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_scat_count += dur.count();
  // Done syncing sys_f, now invert sys_d and apply zero BCs
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
//#pragma omp  critical(initEY)
    { S->Init(E,Y); }// Applies zero BCs and does initial f=Kr
  };
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_solv_count += dur.count();
  // sync sys_f=Kr
  start = std::chrono::high_resolution_clock::now();
#pragma omp single
{   this->halo_val = 0.0; }// serial halo_vals zero
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_gat0_count +=dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic update
        this->halo_val[f+j] += S->sys_f[d*i +j]; };
    };
  };
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_gat1_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static) reduction(+:glob_r2a)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->sys_f[d*i +j] = this->halo_val[f+j]; };
    };
//#pragma omp critical(init)
  S->Init();// looks like not critical
  glob_r2a += S->loca_res2;//FIXME should check this before continuing
  };
//#pragma omp single
#pragma omp for schedule(static) reduction(+:glob_sum1)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH hl0=S->halo_loca_0,sysn=S->udof_n;
    for(INT_MESH i=hl0; i<sysn; i++){
      glob_sum1 += S->sys_d[i] * S->sys_g[i] * S->sys_g[i];
      //glob_chk2 += S->sys_r[i] * S->sys_r[i];
    };
  };
#pragma omp single
{  alpha = glob_r2a / glob_sum1; }// CHECK PRAGMA
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH sysn=S->udof_n;
    for(INT_MESH i=0; i<sysn; i++){
      S->sys_u[i] += alpha * S->sys_p[i];
      S->sys_r[i] -= alpha * S->sys_d[i] * S->sys_g[i];
    };
//#pragma omp atomic read
    S->loca_rto2 = S->loca_rtol*S->loca_rtol *glob_r2a;
#pragma omp atomic write
    glob_to2 = S->loca_rto2;// Pass the relative tolerance out.
  };
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_solv_count += dur.count();
#pragma omp critical(time)
{
  this->time_secs[0]+=float(my_prec_count)*1e-9;
  this->time_secs[1]+=float(my_gmap_count)*1e-9;
  this->time_secs[2]+=float(my_gat0_count)*1e-9;
  this->time_secs[3]+=float(my_gat1_count)*1e-9;
  this->time_secs[4]+=float(my_scat_count)*1e-9;
  this->time_secs[5]+=float(my_solv_count)*1e-9;
}
}// end init parallel region
  this->glob_res2=glob_r2a ;
  this->glob_chk2=glob_r2a ;
  this->glob_rto2=glob_to2 ;
  return 0;
};
int HaloPCR::Iter(){//-----------------------------------------------
  int part_0=0;
  if(std::get<0>( this->mesh_part[0])==NULL ){ part_0=1; };
  const int part_n = int(this->mesh_part.size())-part_0;
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  FLOAT_SOLV glob_sum1=0.0, glob_sum2=0.0, alpha=0.0, beta=0.0;
  FLOAT_SOLV glob_r2a = this->glob_res2;// glob_chk2=0.0,
#pragma omp parallel num_threads(comp_n)
{// iter parallel region
  long int my_phys_count=0, my_scat_count=0, my_solv_count=0,
    my_gat0_count=0,my_gat1_count=0;
  std::chrono::high_resolution_clock::time_point
    start, iter_start;
  auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (start-start);
  iter_start = std::chrono::high_resolution_clock::now();
  start = std::chrono::high_resolution_clock::now();
  //
  std::vector<Mesh::part> P;  P.resize(this->mesh_part.size());
  std::copy(this->mesh_part.begin(), this->mesh_part.end(), P.begin());
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    start = std::chrono::high_resolution_clock::now();
    S->sys_f=0.0;
    E->do_halo=true;
    Y->ElemLinear( E, S->sys_f, S->sys_r );
    dur = std::chrono::duration_cast<std::chrono::nanoseconds>
      (std::chrono::high_resolution_clock::now()-start);
    my_phys_count += dur.count();
    start = std::chrono::high_resolution_clock::now();
    const INT_MESH hnn=E->halo_node_n,hrn=E->halo_remo_n;
    for(INT_MESH i=hrn; i<hnn; i++){
      auto f = d* E->node_haid[i];
      for(uint j=0; j<d; j++){
#pragma omp atomic write
        this->halo_val[f+j] = S->sys_f[d* i+j]; };
    };
    dur= std::chrono::duration_cast<std::chrono::nanoseconds>
      (std::chrono::high_resolution_clock::now()-start);
    my_gat0_count += dur.count();
  };
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic update
        this->halo_val[f+j]+= S->sys_f[d* i+j]; };
    };
  };// End halo_vals sum; now scatter back to elems
  dur= std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_gat1_count += dur.count();
#pragma omp for schedule(static) reduction(+:glob_sum1)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->node_d);
    const INT_MESH hnn=E->halo_node_n;
    start = std::chrono::high_resolution_clock::now();
    for(INT_MESH i=0; i<hnn; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){//NOTE appears not to be critical
#pragma omp atomic read
        S->sys_f[d* i+j] = this->halo_val[f+j]; };// CHECK PRAGMA
    };
    dur= std::chrono::duration_cast<std::chrono::nanoseconds>
      (std::chrono::high_resolution_clock::now()-start);
    my_scat_count += dur.count();
    start = std::chrono::high_resolution_clock::now();
    E->do_halo=false;
    Y->ElemLinear( E, S->sys_f, S->sys_r );
    dur= std::chrono::duration_cast<std::chrono::nanoseconds>
      (std::chrono::high_resolution_clock::now()-start);
    my_phys_count += dur.count();
  };
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static) reduction(+:glob_sum1)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH hl0=S->halo_loca_0,sysn=S->udof_n;
    for(INT_MESH i=hl0; i<sysn; i++){
      glob_sum1 += S->sys_r[i] * S->sys_f[i];
    };
  };
#pragma omp single
{
  beta = glob_sum1 / glob_r2a;
  glob_r2a = glob_sum1;
}
#pragma omp for schedule(static) reduction(+:glob_sum2)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH hl0=S->halo_loca_0,sysn=S->udof_n;
    for(INT_MESH i=0; i<hl0; i++){
      S->sys_p[i] = S->sys_r[i] + beta * S->sys_p[i];
      S->sys_g[i] = S->sys_f[i] + beta * S->sys_g[i];
    };
    for(INT_MESH i=hl0; i<sysn; i++){
      S->sys_p[i] = S->sys_r[i] + beta * S->sys_p[i];
      S->sys_g[i] = S->sys_f[i] + beta * S->sys_g[i];
      glob_sum2  += S->sys_d[i] * S->sys_g[i] * S->sys_g[i];
      //glob_chk2  += S->sys_r[i] * S->sys_r[i];
    };
  };
#pragma omp single
  {  alpha = glob_r2a / glob_sum2; }
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH sysn=S->udof_n;
    for(INT_MESH i=0; i<sysn; i++){
      S->sys_u[i] += alpha * S->sys_p[i];
      S->sys_r[i] -= alpha * S->sys_d[i] * S->sys_g[i]; };
  };
  dur= std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_solv_count += dur.count();
  auto iter_time  = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now() - iter_start);
#pragma omp critical(time)
{
  this->time_secs[0]+=float(my_phys_count)*1e-9;
  this->time_secs[1]+=float(my_gat0_count)*1e-9;
  this->time_secs[2]+=float(my_gat1_count)*1e-9;
  this->time_secs[3]+=float(my_scat_count)*1e-9;
  this->time_secs[4]+=float(my_solv_count)*1e-9;
 //this->time_secs[4]+=float(solv_time.count())*1e-9;
  this->time_secs[5]+=float(iter_time.count())*1e-9;
}
}// end iter parallel region
  this->glob_res2 = glob_r2a;
  this->glob_chk2 = glob_r2a;//FIXME glob_chk2 should be reduction sum of r*r*d
  return 0;
};

