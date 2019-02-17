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
#include "femera.h"
#if VERB_MAX > 0
#include <stdio.h>
#endif
//FIXME Remove unneeded includes
int Mesh::GatherGlobalIDs(){ return 0; };
int Mesh::ScatterHaloIDs(){
  /*
  for(uint i=0; i<this->list_elem.size(); i++){
    if(this->list_elem[i]!=NULL){
      Elem* E=this->list_elem[i];
      //FIXME
  }; };*/
  return 0;
};
int Mesh::SyncIDs(){//FIXME Not parallelized
  //GatherGlobalIDs();
  //std::unordered_map<int,INT_MESH_PART> node_glid_count;// #parts having this node
  //FIXME can use node_glid_parts.size()
  std::unordered_map<int,INT_MESH_PART> node_glid_owner;
  std::unordered_map<int,std::vector<INT_MESH_PART>> node_glid_parts;
  //
  for(uint i=0; i<this->list_elem.size(); i++){
    if(this->list_elem[i]!=NULL){
      Elem* E=this->list_elem[i];// Gather node global IDs ----------
      for(uint j=0; j<E->node_glid.size(); j++){
        auto n = E->node_glid[j];
        node_glid_parts[n].push_back(i);
        //node_glid_count[n]+=1;
        if(node_glid_parts[n].size() == 1){
          node_glid_owner[n] = i; }
        //else if( (E->node_n ==list_elem[node_glid_owner[n]]->node_n)
        //  & (n%2) ){ node_glid_owner[n] = i; }// simple balancing
        //NOTE May be better to leave nodes lumped together, not balanced.
        //NOTE This may reduce the number of mpi messages.
        else if( E->node_n < list_elem[node_glid_owner[n]]->node_n ){
            node_glid_owner[n] = i; };
      };// Gather boundary conditions by global node number ---------
      INT_MESH n,f; FLOAT_MESH v;
      for(auto t : E->rhs_vals){
        std::tie(n,f,v)=t;
        this->rhs_vals.insert(Mesh::nfval( E->node_glid[n],f,v ));
      };
      for(auto t : E->bcs_vals){
        std::tie(n,f,v)=t;
        this->bcs_vals.insert(Mesh::nfval( E->node_glid[n],f,v ));
#if VERB_MAX>3
        if(verbosity>3){
        printf("BCS: %u(%u),%u: %f\n",n,E->node_glid[n],uint(f),v); };
#endif
      };
      for(auto t : E->bc0_nf){
        std::tie(n,f)=t;
        this->bc0_nf.insert(Mesh::nfitem( E->node_glid[n],f ));
      };
    };
  };
  this->node_n=node_glid_owner.size();
#if VERB_MAX>2
  if(verbosity>2){
    printf("Found %u global nodes...\n",node_n); };
#endif
  //ScatterHaloIDs();//----------------------------------------------
  for(auto t : node_glid_parts){
    int n; std::vector<INT_MESH_PART> ps;
    std::tie( n, ps)=t;
    auto o=node_glid_owner[n];
#if VERB_MAX>3
    if(verbosity>3){
      printf("Node %i in parts (",n);
      for(size_t i=0;i<ps.size();i++){ printf("%u ",ps[i]); };
      printf(") owner: %u\n",o);
    };
#endif
    if(ps.size()>1){// n is a halo node
      for(size_t i=0;i<ps.size();i++){
        auto p=ps[i];
        list_elem[p]->halo_node_n++;//FIXME set these later base on *.size()
        list_elem[p]->halo_glid.push_back(n);
        if(p==o){// My local halo node
          list_elem[p]->halo_loca_n++;
          list_elem[p]->loca_glid.push_back(n);
        }else{// Remote halo node
          list_elem[p]->halo_remo_n++;
          list_elem[p]->remo_glid.push_back(n);
        };
      };
    };
  };
  // Renumber local nodes -------------------------------------------
  for(uint i=0; i<this->list_elem.size(); i++){
    if(this->list_elem[i]!=NULL){
      Elem* E=this->list_elem[i];
      RESTRICT Mesh::ints nnew(E->node_n);
      RESTRICT Mesh::ints enew(E->elem_n);
      std::set<INT_MESH> nset={};
#if VERB_MAX>2
  if(verbosity>2){
    printf("Renumbering nodes in partition %u...\n",i); };
#endif
      for(INT_MESH j=0; j<E->halo_remo_n; j++){// remote halo nodes first
        auto l = E->node_loid[ E->remo_glid[j] ];
        //nnew[j]= l ;
        nnew[l]= j ;
        nset.insert( l );
#if VERB_MAX>3
        if(verbosity>3){
        printf(" %u=>%u ", l,j); };
#endif
      };
#if VERB_MAX>3
        if(verbosity>3){ printf(":"); };
#endif
      for(INT_MESH j=0; j<E->halo_loca_n; j++){// then local halo nodes
        auto l = E->node_loid[ E->loca_glid[j] ];
        //nnew[ E->halo_remo_n+j ] = l ; 
        nnew[ l ] = E->halo_remo_n+j ; 
        nset.insert( l );
#if VERB_MAX>3
        if(verbosity>3){ printf(" %u=>%u ", l,E->halo_remo_n+j); };
#endif
      };
#if VERB_MAX>3
        if(verbosity>3){ printf("|"); };
#endif
      // Fill the interior nodes in this partition,
      // sorted by elems they're in.
      // While we're at it, track the elems
      // containing remote and local halo nodes.
      {// scope
      //std::set<INT_MESH> elremo,elloca,elinsi;
      std::vector<INT_MESH> elremo={},elloca={},elinsi={};
      bool isremo=false,isloca=false;//,isinsi=true;
      Mesh::ints enew_conn(E->elem_conn.size());
      INT_MESH node_i=E->halo_node_n;
      for(INT_MESH j=0; j<E->elem_conn.size(); j++){
        auto n = E->elem_conn[j];
        if( nset.count(n)==0 ){
          nset.insert(n);
          nnew[n]=node_i;
#if VERB_MAX>3
        if(verbosity>3){ printf(" %u=>%u ", n,node_i); };
#endif
          node_i++;
        };
        //E->elem_conn[j]=nnew[n];
        enew_conn[j]=nnew[n];
        if(nnew[n]<E->halo_remo_n){ isremo=true; };
        if(nnew[n]<E->halo_node_n){ isloca=true; };
        if( int(j%E->elem_conn_n)==int(E->elem_conn_n-1) ){
          if(isremo){//FIXED These don't need to be sets now.
            //elremo.insert( j/E->elem_conn_n );
            elremo.push_back( j/E->elem_conn_n );
          }else if(isloca){
            //elloca.insert( j/E->elem_conn_n );
            elloca.push_back( j/E->elem_conn_n );
          }else{
            //elinsi.insert( j/E->elem_conn_n );
            elinsi.push_back( j/E->elem_conn_n );
          };
          isremo=false;isloca=false;//isinsi=true;
        };
      };
      // Renumber elements ------------------------------------------
#if VERB_MAX>2
      if(verbosity>2){
        printf("Renumbering elements in partition %u...\n",i); };
#endif
      auto c=uint(E->elem_conn_n);
      INT_MESH elem_i=0;
      for(auto e : elremo){
        std::copy(&enew_conn[c*e],&enew_conn[c*(e+1)],&E->elem_conn[c*elem_i]);
        elem_i++;
      };
      for(auto e : elloca){
        std::copy(&enew_conn[c*e],&enew_conn[c*(e+1)],&E->elem_conn[c*elem_i]);
        elem_i++;
      };
      E->halo_elem_n=elem_i;
      for(auto e : elinsi){
        std::copy(&enew_conn[c*e],&enew_conn[c*(e+1)],&E->elem_conn[c*elem_i]);
        elem_i++;
      };
      }//end scope
#if VERB_MAX>3
      if(verbosity>3){ printf("\n"); };
#endif
      {// scope
      auto g=E->node_glid;
      auto v=E->vert_coor;
      //Mesh::vals v;//=E->vert_coor;// copy this?
      //v.resize(E->vert_coor.size());
      //std::copy(&E->vert_coor[0], &E->vert_coor[E->vert_coor.size()],
      //  &v[0] );
      for(INT_MESH j=0; j<E->node_n; j++){
        E->node_glid[ nnew[j] ] = g[j];
        E->node_loid[ g[j] ] = nnew[j];
        E->vert_coor[ E->mesh_d* nnew[j]+0 ]=v[ E->mesh_d* j+0 ];
        E->vert_coor[ E->mesh_d* nnew[j]+1 ]=v[ E->mesh_d* j+1 ];
        E->vert_coor[ E->mesh_d* nnew[j]+2 ]=v[ E->mesh_d* j+2 ];
      }; }// end scope
      // Can just clear existing BCs and reapply nodal vals.
      E->rhs_vals={}; E->bcs_vals={}; E->bc0_nf={};
      {// scope
      INT_MESH n,f; FLOAT_MESH v;
      for(auto t : this->rhs_vals ){ std::tie(n,f,v)=t;
        if( E->node_loid.count(n)>0 ){
        E->rhs_vals.insert( Mesh::nfval(E->node_loid[n],f,v) ); };
      };
      for(auto t : this->bcs_vals ){ std::tie(n,f,v)=t;
        if( E->node_loid.count(n)>0 ){
        E->bcs_vals.insert( Mesh::nfval(E->node_loid[n],f,v) ); };
        //printf("BCS: %u(%u),%u: %f\n",n,E->node_glid[n],uint(f),v);
      };
      for(auto t : this->bc0_nf   ){ std::tie(n,f)=t;
        if( E->node_loid.count(n)>0 ){
        E->bc0_nf.insert( Mesh::nfitem(E->node_loid[n],f) ); };
      };
      }// end scope
  };};
  return 0;
};
int Mesh::Setup(){
  INT_ORDER pord=1; int part_0=1; bool is_part=true;//FIXME
  int part_n=int(this->mesh_part.size()) - part_0;
  //
  int dots_mod=0;
  if(verbosity==2){ dots_mod=1;
    if( part_n>(comp_n*4) ){ dots_mod=comp_n; };
  }
  //
  INT_MESH elem_tot=0, halo_elem_tot=0;
  INT_MESH node_tot=0, halo_node_tot=0,halo_remo_tot=0,halo_loca_tot=0;
  INT_MESH halo_udof_tot=0;
  std::string solv_name="";
  //Mesh* M = new SolvePCG();// Gmsh* G;
#pragma omp parallel for schedule(static) num_threads(comp_n)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
  // Read Mesh ======================================================
    std::stringstream ss;
    ss << this->base_name; if(is_part){ ss << "_" << part_i << ".fmr" ;};
    std::string pname = ss.str();
#if VERB_MAX>3
    if(verbosity>3){
    //if(part_n<10){
      std::cout << "Reading " << pname << "..." <<'\n';
    //};
    };
#endif
    Elem* E = new Tet(pord);//FIXME
    E->simd_n = this->simd_n;
    Phys* Y = E->ReadPartFMR(pname.c_str(),false);
    Solv* S;//FIXME can this be passed as an argument and copied?
    switch( solv_meth ){
      case(Solv::SOLV_CG):{
        S=new PCG( E->node_n * Y->ndof_n, this->iter_max, glob_rtol ); 
        break;}
      case(Solv::SOLV_CR):{
        S=new PCR( E->node_n * Y->ndof_n, this->iter_max, glob_rtol ); 
        break;}
      default:{ S=new PCG(E->node_n * Y->ndof_n, this->iter_max, glob_rtol );  }
    };
    //S->tol = this->glob_rtol;
    if(dots_mod>0){
      if((part_i%dots_mod)==0){ std::cout <<"."; fflush(stdout); }; };
    E->Setup();
    Y->Setup( E );;// Applies material rotations & sets Y->elem_flop and elem_band
    S->Setup( E,Y );// Applies BCs & sets S->udof_flop and 
    // For PCR, also computes diagonal of K for Jacobi preconditioner
#if VERB_MAX>10
    if(verbosity>10){
#pragma omp critical(print)
{
    uint d=uint(Y->ndof_n);
    for(INT_MESH i=0; i<E->node_n; i++){
      std::cout << E->node_glid[i] <<":";
      for(uint j=0; j<d; j++){
        std::cout <<" "<< S->sys_d[d* i+j];
      }; std::cout <<'\n';
    };
}
    };
#endif
    Mesh::part P(E,Y,S);
#pragma omp critical(systot)
{
    //this->meth_name=S->meth_name;
    this->mesh_part[part_i] = P;
    //M->list_elem[part_i] = E;
    this->elem_n += E->elem_n;
    this->node_n += E->node_n - E->halo_remo_n;
    this->udof_n += uint(Y->ndof_n) * (E->node_n - E->halo_remo_n);
    this->solv_flop += float(S->udof_flop *uint(Y->ndof_n) * (E->node_n - E->halo_remo_n));
    this->solv_band += float(S->udof_band *uint(Y->ndof_n) * (E->node_n - E->halo_remo_n));
    this->phys_flop += float(Y->tens_flop);
    this->phys_band += float(Y->tens_band);
}
  };// End parallel read & setup ====================================
  //this->udof_n = sys_udof_n;
  //read_done = std::chrono::high_resolution_clock::now();
  if(dots_mod>0){ std::cout <<'\n'; };
  //for(uint i=0; i<M->list_elem.size(); i++){
  for(uint i=0; i<this->mesh_part.size(); i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=this->mesh_part[i];
    //std::cout << "Partition " << i ;
    if(verbosity>2){ printf("Partition %u", i); };
    if( E == NULL ){
      if(verbosity>2){ printf(" is null.\n"); };
    }else{
#if VERB_MAX>2
      if(verbosity>2){
        printf(":%10u Elems (%u halo)\n",E->elem_n, E->halo_elem_n);
        printf("            %10u Nodes (%u halo: %u remote, %u local)\n",
          E->node_n, E->halo_node_n, E->halo_remo_n, E->halo_loca_n);
#if VERB_MAX>4
      if(verbosity>4){
        printf("|J|: ");
        for(uint j=9; j<E->elip_jacs.size(); j+=10){
          printf("%+9.2e ",E->elip_jacs[j]); }; printf("\n");
      };
#endif
      if(Y->mtrl_prop.size()>2){
        printf("Orthotropic Elastic: E=%9.2e, nu=%4.2f, G:%9.2e\n",
          Y->mtrl_prop[0], Y->mtrl_prop[1], Y->mtrl_prop[2] );
      }else{
        printf("  Isotropic Elastic: E=%9.2e, nu=%4.2f, G:iso\n",
          Y->mtrl_prop[0], Y->mtrl_prop[1] );
      };
      printf("   Tensor Constants:");
      for(uint i=0; i<Y->mtrl_matc.size(); i++){
        printf("%9.2e",Y->mtrl_matc[i]);
        if( ((i+0)%3)==2){ printf("\n");
        if( i<8 ){ printf("                    "); };
        };
      };
      if(Y->mtrl_dirs.size()>2){
      printf("            Rotated: [%+6.2f,%+6.2f,%+6.2f] rad around [z,x,z].\n",
        Y->mtrl_dirs[0],Y->mtrl_dirs[1],Y->mtrl_dirs[2] ); };
      //std::cout <<"Elastic Material: "
      //  << Y->mtrl_prop[0] <<", "<< Y->mtrl_prop[1] <<'\n';
      //std::cout <<"         Rotated: "
      //  << Y->mtrl_dirs[0] <<", "<< Y->mtrl_dirs[1]
      //  <<", "<< Y->mtrl_dirs[2] <<'\n';
      //std::cout <<"  Preconditioner: ";
      //for(size_t i=0;i<S->sys_d.size();i++){ std::cout << S->sys_d[i] <<" ";};
      //std::cout <<'\n';
    };
#endif
    solv_name      = S->meth_name;
    elem_tot      += E->elem_n;
    halo_elem_tot += E->halo_elem_n;
    node_tot      += E->node_n;
    halo_node_tot += E->halo_node_n;
    halo_remo_tot += E->halo_remo_n;
    halo_loca_tot += E->halo_loca_n;
    halo_udof_tot += E->halo_node_n * Y->ndof_n;
    };
  };
  this->halo_val.resize(halo_udof_tot);//ndof_n*halo_loca_tot);
#if VERB_MAX>1
  if(verbosity>1){
    printf(" Total:%10u Elems (%u halo) in %u partitions\n",
      elem_tot, halo_elem_tot, part_n);
    printf("       %10u Nodes (%u halo: %u remote, %u local)\n",
          node_tot, halo_node_tot, halo_remo_tot, halo_loca_tot);
    //printf("Read and set up in %f s.\n", float(read_time.count())*1e-9 );
    //std::cout <<"Solving ";
    //std::cout <<"system: "<<this->_elem_n<<" Elems, "
    //  <<this->node_n<<" Nodes, "<<this->udof_n<<" DOF; "<<'\n'
    //  <<"     to within: "<<rtol<<" relative tolerance,"<<'\n'
    //  <<"or stopping at: "<<iter_max<<" "<<solv_name<<" iterations..."<<'\n';
  };
#endif
  return 0;
};
