#if VERB_MAX > 2
#include <stdio.h>
//#include<iostream>
#endif
//Are these needed?
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctype.h>
#include "femera.h"
//#include "math.hpp"
/*
int Elem::ScatterVert2Elem(  ){
  //const auto elem_d=E->elem_d;
  //elem_vert.resize(elem_n*elem_vert_n*elem_d);
  const uint d=uint(mesh_d);
  const size_t n=elem_vert_n*elem_n;
  for (size_t i=0; i<n; i++){
    int ic=(i/elem_vert_n)*elem_conn_n*d + i%elem_vert_n;
    elem_vert[std::slice(ic,d,elem_vert_n)]
      =node_coor[std::slice(elem_conn[i]*d,d,1)];
  };
  return 0;
};
int Elem::ScatterVert2Elem( Mesh* M ){//FIXME Deprecated?
  //const auto elem_d=E->elem_d;
  elem_vert.resize(elem_n*elem_vert_n*elem_d);
  const size_t n=elem_vert_n*elem_n;
  for (size_t i=0; i<n; i++){
    int ic=(i/elem_vert_n)*elem_conn_n*elem_d + i%elem_vert_n;
    elem_vert[std::slice(ic,elem_d,elem_vert_n)]//FIXME Should be mesh_d
      =M->node_coor[std::slice(elem_conn[i]*elem_d,elem_d,1)];
  };
  return 0;
};*/
int Elem::Setup(){
  //if(elem_vert.size()<uint(mesh_d)*elem_n*uint(elem_vert_n)){
  //  elem_vert.resize( uint(mesh_d)*elem_n*uint(elem_vert_n) ); };
  //this->ScatterVert2Elem();//printf("* A *\n");
  int bad_jacs = this->JacsDets();//printf("* B *\n");
  if( bad_jacs < 0){
    printf("WARNING: %i Negative element Jacobians.\n",-bad_jacs); };
  const RESTRICT Mesh::vals ipws = GaussLegendre( this->elem_p);
  //if(this->elem_p==2){ this->gaus_n=4; };
  //FIXME this->gaus_p is always 1? );//this->gaus_p );
  this->intp_shpf = ShapeFunction(this->elem_p, ipws );
  const RESTRICT Mesh::vals t=ShapeGradient(this->elem_p, ipws );
  this->intp_shpg.resize(t.size());
  //intp_shpg= t;// icc wants resize() first.
  //FIXME Transpose the shape gradients for ElemLinear()
  uint cn=this->elem_conn_n;
  uint Ng=t.size()/this->gaus_n;
  for(uint p=0;p<this->gaus_n;p++){
    for(uint i=0;i<this->elem_d;i++){
      for(uint j=0;j<cn;j++){
        this->intp_shpg[Ng*p +elem_d *j+i ] = t[Ng*p +cn* i+j];
  };};};
  //printf("*** %u %u %u ***\n",uint(elem_p),uint(gaus_p),uint(intp_shpg.size()) );
  return 0;
  };
const RESTRICT Mesh::vals Elem::ShapeGradient( const INT_ORDER pord,
  const RESTRICT Mesh::vals iptw ){//printf("=== gA ===\n");
  const uint d=uint(elem_d)+1;
  const uint np=iptw.size()/d;
  uint ng=0;//=elem_vert_n*uint(elem_d);// printf("*** SIZE ng: %u\n",ng);
  Mesh::vals shpg;//(ng*np);
  FLOAT_MESH p[elem_d];
  this->gaus_weig.resize(np);
  for(uint ip=0; ip<np; ip++ ){
    //for(uint id=0; id<uint(elem_d); id++){ p[id]=iptw[id*d+ip]; };
    for(uint id=0; id<uint(elem_d); id++){ p[id]=iptw[ip*d+id]; };
    this->gaus_weig[ip]=iptw[ip*d+uint(elem_d)];
    auto g=ShapeGradient(pord, p);// printf("*** SIZE sg: %u\n",uint(g.size()));
    if(ip==0){ ng = uint(g.size()); shpg.resize(ng*np); };
    shpg[std::slice(ng*ip,ng,1)]=g;
  };
  return(shpg);
};

const RESTRICT Mesh::vals Elem::ShapeFunction( const INT_ORDER pord,
  const RESTRICT Mesh::vals iptw ){//printf("=== gA ===\n");
  const uint d=uint(elem_d)+1;
  const uint np=iptw.size()/d;
  uint ng=0;//=elem_vert_n*uint(elem_d);// printf("*** SIZE ng: %u\n",ng);
  Mesh::vals shpf;//(ng*np);
  FLOAT_MESH p[elem_d];
  this->gaus_weig.resize(np);
  for(uint ip=0; ip<np; ip++ ){
    //for(uint id=0; id<uint(elem_d); id++){ p[id]=iptw[id*d+ip]; };
    for(uint id=0; id<uint(elem_d); id++){ p[id]=iptw[ip*d+id]; };
    this->gaus_weig[ip]=iptw[ip*d+uint(elem_d)];
    auto g=ShapeFunction(pord, p);// printf("*** SIZE sg: %u\n",uint(g.size()));
    if(ip==0){ ng = uint(g.size()); shpf.resize(ng*np); };
    shpf[std::slice(ng*ip,ng,1)]=g;
  };
  return(shpf);
};
int Elem::Jac1Dets(){//FIXME 1D can be optimized
  int ok=0;
  /*
  const RESTRICT Mesh::vals ipws = GaussLegendre( (INT_ORDER)1 );//this->gaus_p );
  const RESTRICT Mesh::vals shpg=ShapeGradient(1, ipws );
  //const int cn=elem_vert.size()/elem_n;
  const uint cn=elem_conn_n;
  const uint vn=elem_vert_n;
  const int np=ipws.size()/(elem_d+1);
  const int ng=intp_shpg.size()/np;
  const int d=1, d2=elem_d*elem_d, Nj=2;
  const RESTRICT Mesh::vals iwts=ipws[std::slice(ipws.size()-np,np,1)];
  //
  //elip_dets.resize(elem_n*np);
  elip_jacs.resize(elem_n*np*Nj);
  for(INT_MESH ie=0; ie<elem_n; ie++){
    RESTRICT Mesh::vals vert(vn*d);
    for(uint i=0; i<vn; i++){ uint n=elem_conn[cn* ie+i];
      for(uint j=0;j<d;j++){
        vert[vn* j+i]=node_coor[d* n+j]; }; };
    for(int ip=0; ip<np; ip++){
      //RESTRICT Mesh::vals jac=MatMul2xNx2T(
      RESTRICT Mesh::vals jac={inner_product( vert,
        //elem_vert[std::slice(ie*cn,cn,1)],
        intp_shpg[std::slice(ip*ng,ng,1)] )};
      FLOAT_MESH det=Jac2Det(jac);//FIXME just a scalar...
      //int ok_jac=Jac2Inv(jac,det)//FIXME return these?
      Jac2Inv(jac,det);//FIXME just a scalar...
      if(det<=0){ok=-1;};
      //elip_dets[ie*np+ip]=det*iwts[ip];//printf("DETWT: %10.3e\n",elip_dets[ie*np+ip]);
      elip_jacs[std::slice(ie*np*d2+ip*d2,d2,1)]=jac;
      //for(int i=0;i<4;i++){printf(" %g",jac[i]);};printf("\n");
      };
    };
    */
  return ok; };
int Elem::Jac2Dets(){
  int ok=0;
  const RESTRICT Mesh::vals ipws = GaussLegendre( 1 );//this->gaus_p );
  //auto p=this->elem_p; this->elem_p=1;
  const RESTRICT Mesh::vals shpg=ShapeGradient(1, ipws );
  //this->elem_p=p;
  //const int cn=elem_vert.size()/elem_n;
  const uint cn=elem_conn_n;
  const uint vn=elem_vert_n;
  const int np=ipws.size()/(elem_d+1);
  const int ng=shpg.size()/np;
  const int d=2;
  const int d2=elem_d*elem_d, Nj=d2+1;
  FLOAT_MESH jac[d2], vert[vn*d];
  //const RESTRICT Mesh::vals iwts=ipws[std::slice(ipws.size()-np,np,1)];
  //const RESTRICT Mesh::vals iwts=ipws[std::slice(d,np,d+1)];
  //printf("IPWS: ");
  //for(size_t i=0; i<iwts.size(); i++){
  //  printf("%+10.3e ",iwts[i]);
  //};printf("\n");
  //
  //elip_dets.resize(elem_n*np);
  elip_jacs.resize(elem_n*np*Nj);
  for(INT_MESH ie=0; ie<elem_n; ie++){//printf("Triangle Jacobian Elem %i\n",ie);
    //RESTRICT Mesh::vals vert(vn*d);
    for(uint i=0; i<vn; i++){ uint n=elem_conn[cn* ie+i];
      for(uint j=0;j<d;j++){
        vert[vn* j+i]=node_coor[d* n+j]; }; };
        //vert[d* i+j]=node_coor[d* n+j]; }; };
    for(int ip=0; ip<np; ip++){
      //RESTRICT Mesh::vals jac(d2);
      //for( int i=0;i<d2;i++){ jac[i]=0.0; };
      for(uint i=0;i< d;i++){
      for(uint k=0;k< d;k++){ jac[d* i+k ]=0.0;
      for(uint j=0;j<vn;j++){
        jac[d* i+k ] += shpg[ip*ng+ vn* i+j ] * vert[vn* k+j ];
        //jac[d* i+k ] += shpg[ip*ng+ d* j+i ] * vert[d* j+k ];
      };};};
      //RESTRICT Mesh::vals jac=MatMul2xNx2T(
      //  shpg[std::slice(ip*ng,ng,1)], vert );
      //  //elem_vert[std::slice(ie*cn,cn,1)] );
      FLOAT_MESH det=Jac2Det(jac);
      //int ok_jac=Jac2Inv(jac,det)//FIXME return these?
      Jac2Inv(jac,det);
      if(det<=0){ok-=1;};
      //elip_dets[ie*np+ip]=det*iwts[ip];//printf("DETWT: %10.3e\n",elip_dets[ie*np+ip]);
      //elip_jacs[std::slice(ie*np*Nj+ip*Nj,d2,1)]=jac;
      for(int i=0;i<d2;i++){ elip_jacs[ie*np*Nj+ip*Nj +i ] = jac[i]; };
      elip_jacs[ie*np*Nj+ip*Nj+d2]=det;//*iwts[ip];
      //for(int i=0;i<4;i++){printf(" %g",jac[i]);};printf("\n");
      };
    };
  return ok; };
int Elem::Jac3Dets(){
  int ok=0;// printf("**** JA ****\n");
  const RESTRICT Mesh::vals ipws = GaussLegendre( (INT_ORDER)1 );//this->gaus_p );
  //auto p=this->elem_p;
  //this->elem_p=(INT_ORDER)1;
  //printf("**** JB %u ****\n",uint(ipws.size()));
  //intp_shpg.resize(vert_n * uint(elem_d));
  const RESTRICT Mesh::vals shpg=ShapeGradient((INT_ORDER)1, ipws );
  //RESTRICT Mesh::vals t(12);
  //t=ShapeGradient( ipws ); 
  //intp_shpg.resize(t.size()); intp_shpg= t;// icc wants resize() before assign..
  //printf("**** JC %u ****\n",uint(intp_shpg.size()) );
  //this->elem_p=p;// printf("**** JD ****\n");
  const uint d = 3, d2=d*d, Nj=d2+1;//int(elem_d);
  //const int cn=elem_vert.size()/elem_n;
  const uint cn=elem_conn_n;
  const uint vn=elem_vert_n;//*mesh_d;
  const uint np=ipws.size()/(d+1);
  const uint ng=shpg.size()/np;// printf("**** JE ****\n");
  FLOAT_MESH jac[d2], vert[vn*d];
  //const int d2=elem_d*elem_d;
  //const int d2=9,Nj=10;
  //const RESTRICT Mesh::vals iwts=ipws[std::slice(d,np,d+1)];
  //elip_dets.resize(elem_n*np);
  elip_jacs.resize(elem_n*np*Nj);//printf("ELEMS:%u, INTPTS: %i\n",elem_n,np);
  for(INT_MESH ie=0; ie<elem_n; ie++){// printf("**** JF ****\n");
    //RESTRICT Mesh::vals vert(vn*d);
    for(uint i=0; i<vn; i++){ uint n=elem_conn[cn* ie+i];
      for(uint j=0;j<d;j++){
        vert[vn* j+i]=node_coor[d* n+j]; }; };
        //vert[d* i+j]=node_coor[d* n+j]; }; };
    for(uint ip=0; ip<np; ip++){//printf("ELEM:%u, INTPT: %i\n",ie,ip);
      //RESTRICT Mesh::vals jac(0.0,d2);
      //for( int i=0;i<d2;i++){ jac[i]=0.0; };
      for(uint i=0;i< d;i++){
      for(uint k=0;k< d;k++){ jac[d* i+k ]=0.0;
      for(uint j=0;j<vn;j++){
        jac[d* i+k ] += shpg[ip*ng+ vn* i+j ] * vert[vn* k+j ];
        //jac[d* i+k ] += shpg[ip*ng+ d* j+i ] * vert[d* j+k ];
      };};};
      //RESTRICT Mesh::vals jac=MatMul3xNx3T(//FIXME loop this
      //  shpg[std::slice(ip*ng,ng,1)], vert );
      //  //elem_vert[std::slice(ie*cn*d,cn*d,1)] );
      FLOAT_MESH det=Jac3Det(jac);//printf("DET: %e\n",det);
      //int ok_jac=Jac3Inv(jac,det)//FIXME return these?
      Jac3Inv(jac,det);
      if(det<=0){ok-=1;};
      for(uint i=0;i<d2;i++){ elip_jacs[ie*np*Nj+ip*Nj +i ] = jac[i]; };
      //elip_dets[ie*np+ip]=det*iwts[ip];
      //elip_jacs[std::slice(ie*np*Nj+ip*Nj,d2,1)]=jac;// printf("**** JG ****\n");
      //elip_jacs[ie*np*Nj+ip*Nj +d2 ]=det*iwts[ip];//FIXED Don't apply weight yet
      elip_jacs[ie*np*Nj+ip*Nj +d2 ]=det;
      #if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(size_t j=0;j<Nj;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e",elip_jacs[ie*np*Nj+ip*Nj +j ]);
      }; printf("\n");
      #endif
      };
    };
  return ok;
};
//

