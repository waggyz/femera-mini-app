#if VERB_MAX > 2
#include <stdio.h>
//#include<iostream>
#endif
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
      =vert_coor[std::slice(elem_conn[i]*d,d,1)];
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
      =M->vert_coor[std::slice(elem_conn[i]*elem_d,elem_d,1)];
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
  const RESTRICT Mesh::vals t=ShapeGradient(this->elem_p, ipws );
  this->intp_shpg.resize(t.size()); intp_shpg= t;// icc wants resize() first.
  //printf("*** %u %u %u ***\n",uint(elem_p),uint(gaus_p),uint(intp_shpg.size()) );
  return 0;
  };
const RESTRICT Mesh::vals Elem::ShapeGradient( const INT_ORDER pord,
  const RESTRICT Mesh::vals iptw ){//printf("=== gA ===\n");
  const uint d=uint(elem_d)+1;
  const uint np=iptw.size()/d;
  uint ng=elem_vert_n*uint(elem_d);// printf("*** SIZE ng: %u\n",ng);
  Mesh::vals shpg(ng*np);
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
        vert[vn* j+i]=vert_coor[d* n+j]; }; };
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
    RESTRICT Mesh::vals vert(vn*d);
    for(uint i=0; i<vn; i++){ uint n=elem_conn[cn* ie+i];
      for(uint j=0;j<d;j++){
        vert[vn* j+i]=vert_coor[d* n+j]; }; };
    for(int ip=0; ip<np; ip++){
      RESTRICT Mesh::vals jac(d2);
      for(uint j=0;j<vn;j++){
        for(uint i=0;i<d;i++){
          for(uint k=0;k<d;k++){
            jac[d* i+k ] += shpg[ip*ng+ vn* i+j ] * vert[vn* k+j ];
      };};};
      //RESTRICT Mesh::vals jac=MatMul2xNx2T(
      //  shpg[std::slice(ip*ng,ng,1)], vert );
      //  //elem_vert[std::slice(ie*cn,cn,1)] );
      FLOAT_MESH det=Jac2Det(jac);
      //int ok_jac=Jac2Inv(jac,det)//FIXME return these?
      Jac2Inv(jac,det);
      if(det<=0){ok-=1;};
      //elip_dets[ie*np+ip]=det*iwts[ip];//printf("DETWT: %10.3e\n",elip_dets[ie*np+ip]);
      elip_jacs[std::slice(ie*np*Nj+ip*Nj,d2,1)]=jac;
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
  //const int d2=elem_d*elem_d;
  //const int d2=9,Nj=10;
  //const RESTRICT Mesh::vals iwts=ipws[std::slice(d,np,d+1)];
  //elip_dets.resize(elem_n*np);
  elip_jacs.resize(elem_n*np*Nj);//printf("ELEMS:%u, INTPTS: %i\n",elem_n,np);
  for(INT_MESH ie=0; ie<elem_n; ie++){// printf("**** JF ****\n");
    RESTRICT Mesh::vals vert(vn*d);
    for(uint i=0; i<vn; i++){ uint n=elem_conn[cn* ie+i];
      for(uint j=0;j<d;j++){
        vert[vn* j+i]=vert_coor[d* n+j]; }; };
        //vert[d* i+j]=vert_coor[d* n+j]; }; };
    for(uint ip=0; ip<np; ip++){//printf("ELEM:%u, INTPT: %i\n",ie,ip);
      RESTRICT Mesh::vals jac(0.0,d2);
      for(uint i=0;i<d;i++){
      for(uint k=0;k<d;k++){// jac[d* i+k ]=0.0;
      for(uint j=0;j<vn;j++){
        jac[d* i+k ] += shpg[ip*ng+ vn* i+j ] * vert[vn* k+j ];
      };};};
      //RESTRICT Mesh::vals jac=MatMul3xNx3T(//FIXME loop this
      //  shpg[std::slice(ip*ng,ng,1)], vert );
      //  //elem_vert[std::slice(ie*cn*d,cn*d,1)] );
      FLOAT_MESH det=Jac3Det(jac);//printf("DET: %e\n",det);
      //int ok_jac=Jac3Inv(jac,det)//FIXME return these?
      Jac3Inv(jac,det);
      if(det<=0){ok-=1;};
      //elip_dets[ie*np+ip]=det*iwts[ip];
      elip_jacs[std::slice(ie*np*Nj+ip*Nj,d2,1)]=jac;// printf("**** JG ****\n");
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
Phys* Elem::ReadPartFMR( const char* fname, bool is_bin ){
  Phys* Y = new ElastIso3D( 100e9,0.3 );// default physics
  //
  std::string s; if(is_bin){ s="binary";}else{s="ASCII";};
  if(is_bin){
    std::cout << "ERROR Could not open "<< fname << " for reading." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return Y;}
  //std::cout <<fname;
  //int eltype=4;//FIXME element type id
  //int elem_p=pord;//FIXME
  //
  std::string fmrstring;
  std::ifstream fmrfile(fname);//return 0;
  while( fmrfile >> fmrstring ){// std::cout <<fmrstring;//printf("%s ",fmrstring.c_str());
    if(fmrstring=="$Femera"){
      std::string line;
      std::getline(fmrfile, line);//FIXME Does not remove this line from stream.
      //FIXME just ignore this for now
      //fmrfile >> version >> file_type >> data_size ;
    };
    if(fmrstring=="$Elem"){
      int eltype,d,p;//FIXME create new Elem* based on eltype.
      fmrfile >> d >> eltype >> p;// this=new Tet(p);
      this->elem_d=(INT_DOF)d;
      this->elem_p =(INT_ORDER)p;
      //std::cout << uint(elem_d) << eltype << uint(elem_p) <<'\n';
    };
    if(fmrstring=="$Conn"){
      int c;
      fmrfile >> c >> this->elem_n >> this->halo_elem_n ;
      this->elem_conn_n =(INT_ELEM_NODE)c;
      //std::cout <<"***"<< uint(elem_conn_n) << elem_n << halo_elem_n <<'\n';
      this->elem_conn.resize(uint(elem_conn_n)*elem_n);
      elem_glid.resize(elem_n);
      for(uint i=0; i<this->elem_n; i++){
        fmrfile >> elem_glid[i];
        for(uint j=0; j<uint(c);j++){ fmrfile >> elem_conn[uint(c)* i+j]; };
      };
    };
    if(fmrstring=="$Node"){
      int d,v;//return 0;
      fmrfile >> d >> v >> this->node_n
        >> this->halo_node_n >> this->halo_remo_n >> this->halo_loca_n ;
      //fmrfile >> d >> d >>d>>d>>d>>d;
      this->mesh_d=(INT_ORDER)d; vert_n=v;
      //this->vert_n=node_n;
      //return 0;//(INT_MESH )v;return 0;
      //std::cout << uint(mesh_d) << vert_n << node_n << halo_node_n <<'\n';
    };/*
    if(fmrstring=="$HaloNodeID"){
      node_glid.resize(halo_node_n);
      for(size_t i=0; i<node_glid.size(); i++){
        fmrfile >> node_glid[i]; 
        node_loid[node_glid[i]] = i;
      };*/
    /*};
    if(fmrstring=="$HaloRemote"){
      halo_remo.resize(halo_remo_n);
      for(size_t i=0; i<halo_remo.size(); i++){
        fmrfile >> halo_remo[i];
      };
    };
    if(fmrstring=="$HaloLocal"){
      halo_loca.resize(halo_loca_n);
      for(size_t i=0; i<halo_loca.size(); i++){
        fmrfile >> halo_loca[i];
      };
    };*/
    if(fmrstring=="$VertCoor"){
      vert_coor.resize(uint(mesh_d)*node_n);//FIXME vert_n
      node_glid.resize(node_n);
      //for(size_t i=0; i<vert_coor.size(); i++){
      //  fmrfile >> vert_coor[i]; };
      for(INT_MESH i=0; i<node_n; i++){//FIXME vert_n
        fmrfile >> node_glid[i];
        node_loid[node_glid[i]] = i;
        for(uint j=0; j<(uint)this->mesh_d; j++){
          fmrfile >> vert_coor[this->mesh_d *i+j]; };//return 0;
      };
    };
    if(fmrstring=="$BC0"){ INT_MESH n,f,m;
      fmrfile >> m;
      for(INT_MESH i=0; i<m; i++){
         fmrfile >> n >> f ; 
        bc0_nf.insert(Mesh::nfitem(n,f));
      };
    };
    if(fmrstring=="$BCS"){ INT_MESH n,f,m; FLOAT_SOLV v;
      fmrfile >> m;
      for(INT_MESH i=0; i<m; i++){
         fmrfile >> n >> f >> v; 
        bcs_vals.insert(Mesh::nfval(n,f,v));
      };
    };
    if(fmrstring=="$RHS"){ INT_MESH n,f,m; FLOAT_SOLV v;
      fmrfile >> m;
      for(INT_MESH i=0; i<m; i++){
         fmrfile >> n >> f >> v; 
        rhs_vals.insert(Mesh::nfval(n,f,v));
      };
    };
    if(fmrstring=="$ElasticProperties"){
      Phys::vals mtrl_prop={},mtrl_dirs={};
      int s=0; fmrfile >> s;
      mtrl_prop.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i]; };
      s=0; fmrfile >> s;
      if(s>0){
        mtrl_dirs.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> mtrl_dirs[i]; };
      // mtrl_dirs[i]*=(PI/180.0) ;};
      };
      if(mtrl_dirs.size()<3){
        Y = new ElastIso3D(mtrl_prop[0],mtrl_prop[1]);
      }else{
        Y = new ElastOrtho3D(mtrl_prop,mtrl_dirs);
      };
      //Y->MtrlProp2MatC();
    };
    /*
    //if(fmrstring=="$Physics"){ //FIXME
    if(fmrstring=="$ElasticIsotropic"){ //FIXME
      fmrfile >> m;// Should be 2
      //for(INT_MESH i=0; i<m; i++){
         fmrfile >> young >> poiss ; 
      //};
    };
    if(fmrstring=="$ElasticOrthotropic"){ //FIXME
    };
    */
    //std::cout <<"*" << fmrstring <<"*" ;
    };//EOF
  //
  return Y;}
//-------------------------------------------------------------------
int Elem::SavePartFMR( const char* fname, bool is_bin ){
  //std::stringstream ss;
  //
  std::string s; if(is_bin){ s="binary";}else{s="ASCII";};
  if(is_bin){
    std::cout << "ERROR Could not save "<< fname << "." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return 1;}
  //
  int eltype=4;//FIXME element type id
  //int elem_p=pord;//FIXME
  //
  std::ofstream fmrfile;
  fmrfile.open(fname);
  //
  fmrfile << "$Femera" <<'\n';
  fmrfile <<""<< ((FLOAT_MESH)0.1) <<" "<< int(is_bin)
    <<" "<< sizeof(INT_MESH) <<" "<< sizeof(INT_DOF)
    <<" "<< sizeof(FLOAT_MESH)<<" "<< sizeof(FLOAT_SOLV)
    <<" "<< ((INT_MESH)1) <<'\n';
  fmrfile << "$Elem" <<'\n';
  fmrfile <<""<< int(elem_d) <<" "<< eltype <<" "<< int(elem_p) <<'\n';
  fmrfile << "$Conn" <<'\n';
  fmrfile <<""<< int(elem_conn_n) <<" "<< elem_n <<" "<< halo_elem_n <<'\n';
  for(INT_MESH i=0; i<elem_n; i++){
    fmrfile << elem_glid[i];
    for(uint j=0; j<uint(elem_conn_n); j++){
      fmrfile << " " << elem_conn[uint(elem_conn_n)*i +j];
    };
    fmrfile<<'\n';
  };
  
  //for(size_t i=0; i<elem_conn.size(); i++){
  //  if(!(i%elem_conn_n)){ fmrfile<<'\n'; };
  //  fmrfile << " " << elem_conn[i]; }; fmrfile<<'\n';
    
    
  fmrfile << "$Node" <<'\n';
  //halo_uniq_n=halo_uniq.size();
  //E->halo_loca_n=halo_coor.size();
  fmrfile <<""<< int(mesh_d) <<" "<< vert_n <<" "<< node_n
    <<" "<< this->halo_node_n <<" "<< halo_remo_n<<" "<< this->halo_loca_n <<'\n';
  /*
  if(node_glid.size()>0){//E->halo_node_n){
    fmrfile << "$HaloNodeID" ;
    for(size_t i=0; i<node_glid.size(); i++){
      if(!(i%20)){ fmrfile<<'\n'; };
      fmrfile << " " << node_glid[i]; }; fmrfile<<'\n';
  };*/
  /*
  if(halo_loca.size()>0){
    fmrfile << "$HaloLocal" ;
    for(size_t i=0; i<halo_loca.size(); i++){
      if(!(i%20)){ fmrfile<<'\n'; };
      fmrfile << " " << halo_loca[i]; }; fmrfile<<'\n';
  };*/
  fmrfile << "$VertCoor"<<'\n';;
  for(INT_MESH i=0; i<this->vert_n; i++){
    fmrfile << this->node_glid[i];
    for(uint j=0; j<(uint)this->mesh_d; j++){
      fmrfile <<" "<< vert_coor[this->mesh_d* i+j]; };
    fmrfile <<'\n';
  };
  if(bc0_nf.size()>0){
    fmrfile << "$BC0" <<'\n'<<""<< bc0_nf.size() <<'\n';
    for(auto i : bc0_nf ){ int n,f; std::tie(n,f)=i;
      fmrfile << "" << n <<" "<< f <<'\n'; };
  };
  if(bcs_vals.size()>0){
    fmrfile << "$BCS" <<'\n'<<""<< bcs_vals.size() <<'\n';
    for(auto i : bcs_vals ){ int n,f; FLOAT_MESH v; std::tie(n,f,v)=i;
      fmrfile << "" << n <<" "<< f <<" "<< v <<'\n'; };
  };
  if(rhs_vals.size()>0){
    fmrfile << "$RHS" <<'\n'<<""<< rhs_vals.size() <<'\n';
    for(auto i : rhs_vals ){ int n,f; FLOAT_MESH v; std::tie(n,f,v)=i;
      fmrfile << "" << n <<" "<< f <<" "<< v <<'\n'; };
  };
  //
  fmrfile.close();
  //};
  return 0;}
//-------------------------------------------------------------------
int Elem::ReadPartCSV( const char* bname ){
  //
  //std::string fname;
  char delim = ','; // File delimiting character
  std::string tok, line;
  std::stringstream ss;//, ls;
  //
  std::vector<FLOAT_MESH> vf={};
  std::vector<  INT_MESH> vi={};
  //
  ss.str(std::string()); vf={}; vi={};
  ss << bname << ".xyz" << ".csv" ;
  //fname = ss.str();
  { std::ifstream csvfile(ss.str());
    while (std::getline(csvfile, line)) {
      std::stringstream ls(line);
      while (std::getline(ls, tok, delim)) {
        vf.push_back(stof(tok));
    }; };
  }
  this->vert_coor.resize(vf.size());
  for(size_t i=0; i<vf.size(); i++){ this->vert_coor[i] = vf[i]; };
  //std::copy(&vf[0], &vf[vf.size()], &this->vert_coor[0]);
  //std::copy(vf.begin(), vf.end(), &this->vert_coor[0]);
  //
  ss.str(std::string()); vf={}; vi={};
  ss << bname << ".nid" << ".csv" ;
  //fname = ss.str();
  { std::ifstream csvfile(ss.str());
    while (std::getline(csvfile, line)) {
      std::stringstream ls(line);
      while (std::getline(ls, tok, delim)) {
        vi.push_back(stoi(tok));// std::cout << line <<" * ";
      }; };
  }
  this->node_glid.resize(vi.size());
  for(size_t i=0; i<vi.size(); i++){
    this->node_glid[i] = vi[i];
    this->node_loid[vi[i]] = i;
  };
  //std::copy(&vi[0], &vi[vi.size()], &this->node_glid[0]);
  auto halo_n=this->node_glid.size();
  //
  INT_MESH halo_e=0;
  ss.str(std::string()); vf={}; vi={};
  ss << bname << ".tet" << ".csv" ;
  //fname = ss.str();
  { std::ifstream csvfile(ss.str()); INT_MESH ci=0;
    while (std::getline(csvfile, line)) {
      std::stringstream ls(line);// std::cout << line <<" | ";
      while (std::getline(ls, tok, delim)) {
        INT_MESH n=stoi(tok);
        vi.push_back(n);// std::cout << tok <<" * ";
        if(n<halo_n){ halo_e=ci/uint(this->elem_conn_n); };
        ci++;// Find the last element containing a halo node.
    }; };
  }
  this->elem_conn.resize(vi.size());
  for(size_t i=0; i<vi.size(); i++){ this->elem_conn[i] = vi[i]; };
  //std::copy(&vi[0], &vi[vi.size()], &this->elem_conn[0]);
  //
  ss.str(std::string());
  ss << bname << ".rhs" << ".csv" ;
  //fname = ss.str();
  { std::ifstream csvfile(ss.str());
    INT_MESH n; INT_DOF f; FLOAT_SOLV v;// std::vector<Mesh::nfval> t={};
    while (std::getline(csvfile, line)) {
      std::stringstream ls(line);
      if(std::getline(ls, tok, delim)){ n=stoi(tok); };
      if(std::getline(ls, tok, delim)){ f=(INT_DOF)stoi(tok); };
      if(std::getline(ls, tok, delim)){ v=stof(tok);
        //t.push_back(Mesh::nfval( n,f,v )); };
        this->rhs_vals.insert(Mesh::nfval( n,f,v )); };
    };
  //this->rhs_vals.resize(vi.size());
  //for(size_t i=0; i<vi.size(); i++){ this->rhs_vals[i] = t[i]; };
  }
  //
  ss.str(std::string());
  ss << bname << ".bcs" << ".csv" ;
  //fname = ss.str();
  { std::ifstream csvfile(ss.str());
    INT_MESH n; INT_DOF f; FLOAT_SOLV v;// std::vector<Mesh::nfval> t={};
    while (std::getline(csvfile, line)) {
      std::stringstream ls(line);
      if(std::getline(ls, tok, delim)){ n=stoi(tok); };
      if(std::getline(ls, tok, delim)){ f=(INT_DOF)stoi(tok); };
      if(std::getline(ls, tok, delim)){ v=stof(tok);
        //t.push_back(Mesh::nfval( n,f,v )); };
        this->bcs_vals.insert(Mesh::nfval( n,f,v )); };
    };
  //this->bcs_vals.resize(vi.size());
  //for(size_t i=0; i<vi.size(); i++){ this->bcs_vals[i] = t[i]; };
  }
  //
  ss.str(std::string());
  ss << bname << ".bc0" << ".csv" ;
  //fname = ss.str();
  { std::ifstream csvfile(ss.str());
    INT_MESH n; INT_DOF f;// std::vector<Mesh::nfitem> t={};
    while (std::getline(csvfile, line)) {
      std::stringstream ls(line);
      if(std::getline(ls, tok, delim)){ n=stoi(tok); };
      if(std::getline(ls, tok, delim)){ f=(INT_DOF)stoi(tok);
        //t.push_back(Mesh::nfitem( n,f )); };
        this->bc0_nf.insert(Mesh::nfitem( n,f )); };
    };
  //this->bc0_nf.resize(vi.size());
  //for(size_t i=0; i<vi.size(); i++){ this->bc0_nf[i] = t[i]; };
  }
  //-----------------------------------------------------------------
  this->mesh_d = this->elem_d;
  this->vert_n = this->vert_coor.size() / uint(this->mesh_d);
  this->node_n = this->vert_n + (uint(this->elem_p)-1) * uint(this->elem_edge_n);
  this->elem_n = this->elem_conn.size() / uint(this->elem_conn_n);
  this->halo_node_n = this->node_glid.size();
  this->halo_elem_n = halo_e;
#if VERB_MAX>2
  std::cout << "Read "<< elem_n << " elems with "
    << halo_elem_n << " halo elems, and "
    << node_n << " nodes with " << halo_node_n << " halo nodes." <<'\n';
#endif
  //
  return 0;
};// end ReadPartCSV
//
int Elem::SavePartCSV( const char* bname ){
  //printf("Saving %s...\n",fname);
  std::stringstream ss;
  std::string fname; std::ofstream femfile; uint ll;
  //
  ss << bname << ".xyz" << ".csv" ;
  fname = ss.str();
  //std::cout << "Saving " << fname << "..." <<'\n';
  femfile.open(fname); ll=this->mesh_d;
  for(size_t i=0; i<vert_n; i++){
    for(uint j=0; j<ll; j++){
      femfile << this->vert_coor[ll* i+j ] ;
      if(j<uint(ll-1)){femfile << ',';};
    }; femfile<<'\n';
  }; femfile.close();
  //
  ss.str(std::string());// clear the stringstream
  ss << bname << ".tet" << ".csv" ;
  fname = ss.str();
  //std::cout << "Saving " << fname << "..." <<'\n';
  femfile.open(fname); ll=this->elem_conn_n;
  for(size_t i=0; i<elem_n; i++){
    for(uint j=0; j<ll; j++){
      femfile << this->elem_conn[ll* i+j ] ;
      if(j<uint(ll-1)){femfile << ',';};
    }; femfile<<'\n';
  }; femfile.close();
  //
  INT_MESH n; INT_DOF f; FLOAT_SOLV v;
  if(this->bc0_nf.size()>0){
    ss.str(std::string());
    ss << bname << ".bc0" << ".csv" ;
    fname = ss.str();
    femfile.open(fname);
    //std::cout << "Saving " << fname << "..." <<'\n';
    for(auto t : this->bc0_nf  ){ std::tie(n,f)=t;
      femfile << n << "," << uint(f) <<'\n'; };
    femfile.close();
  };
  if(this->bcs_vals.size()>0){
    ss.str(std::string());
    ss << bname << ".bcs" << ".csv" ;
    fname = ss.str();
    femfile.open(fname);
    //std::cout << "Saving " << fname << "..." <<'\n';
    for(auto t : this->bcs_vals){ std::tie(n,f,v)=t;
      femfile << n << "," << uint(f) << "," << v <<'\n'; };
    femfile.close();
  };
  if(this->rhs_vals.size()>0){
    ss.str(std::string());
    ss << bname << ".rhs" << ".csv" ;
    fname = ss.str();
    femfile.open(fname);
    //std::cout << "Saving " << fname << "..." <<'\n';
    for(auto t : this->rhs_vals){ std::tie(n,f,v)=t;
      femfile << n << "," << uint(f) << "," << v <<'\n'; };
    femfile.close();
  };
  if(this->node_glid.size()>=this->halo_node_n){
    ss.str(std::string());
    ss << bname << ".nid" << ".csv" ;
    fname = ss.str();
    femfile.open(fname);
    for(size_t i=0; i<this->halo_node_n;i++){
      femfile << this->node_glid[i] <<'\n'; };
    femfile.close();
  };
  return 0;
};
