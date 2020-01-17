#include <utility>//std::pair
#include <vector>
#include <set>// This is ordered
#include <algorithm>    // std::copy
#include <unordered_map>
#include <vector>
#include <tuple>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <chrono>
#include "femera.h"
#if VERB_MAX > 0
#include <stdio.h>
#endif

#if OMP_NESTED!=true
std::vector<Mesh::part> Mesh::priv_part;
#endif

//FIXME Remove unneeded includes
int Mesh::GatherGlobalIDs(){ return 0; };
int Mesh::ScatterHaloIDs(){
#if 0
  for(uint i=0; i<this->list_elem.size(); i++){
    if(this->list_elem[i]!=NULL){
      Elem* E=this->list_elem[i];
      //FIXME
  } }
#endif
  return 0;
};
int Mesh::SyncIDs(){//FIXME Not parallelized
  //GatherGlobalIDs();
  //std::unordered_map<int,INT_PART> node_glid_count;// #parts having this node
  //FIXME can use node_glid_parts.size()
  std::unordered_map<int,INT_PART> node_glid_owner;
  std::unordered_map<int,std::vector<INT_PART>> node_glid_parts;
  //
  for(uint i=0; i<this->list_elem.size(); i++){
    if(this->list_elem[i]!=NULL){
      Elem* E=this->list_elem[i];// Gather node global IDs ----------
      for(uint j=0; j<E->node_glid.size(); j++){
        auto n = E->node_glid[j];
        node_glid_parts[n].push_back(i);
        // Assign this node's owner
        if(node_glid_parts[n].size() == 1){// Node has no owner yet
          node_glid_owner[n] = i; }
        else{
          switch(this->hown_meth){
          case(1):{// owner: fewest nodes, first touch tiebreaker
            //NOTE May be better to leave nodes lumped together, not balanced.
            //NOTE This may reduce the number of mpi messages.
            if( E->node_n < list_elem[node_glid_owner[n]]->node_n ){
              // Assign owner to smallest partition (fewest nodes)
              node_glid_owner[n] = i; };
            break ;}
          case(2):{//owner: fewest nodes, even-odd tiebreaker
            if( (E->node_n ==list_elem[node_glid_owner[n]]->node_n)
              // This partition is the same size as the current owner
              & (n%2) ){ node_glid_owner[n] = i; }// even-odd tiebreaker
            else if( E->node_n < list_elem[node_glid_owner[n]]->node_n ){
              // Assign owner to smallest partition (fewest nodes)
              node_glid_owner[n] = i; };
              break; }
          case(3):{// random
              node_glid_owner[n] = node_glid_parts[n][
                (node_glid_parts[n].size() * std::rand())/(long(RAND_MAX)+1) ];
            break; };
          default:{ break; }// also case(0)
          };
        };
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
    int n; std::vector<INT_PART> ps;
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
        list_elem[p]->halo_node_n++;//FIXME set these later based on *.size()
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
      auto v=E->node_coor;
      //Mesh::vals v;//=E->node_coor;// copy this?
      //v.resize(E->node_coor.size());
      //std::copy(&E->node_coor[0], &E->node_coor[E->node_coor.size()],
      //  &v[0] );
      for(INT_MESH j=0; j<E->node_n; j++){
        E->node_glid[ nnew[j] ] = g[j];
        E->node_loid[ g[j] ] = nnew[j];
        E->node_coor[ E->mesh_d* nnew[j]+0 ]=v[ E->mesh_d* j+0 ];
        E->node_coor[ E->mesh_d* nnew[j]+1 ]=v[ E->mesh_d* j+1 ];
        E->node_coor[ E->mesh_d* nnew[j]+2 ]=v[ E->mesh_d* j+2 ];
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
  int part_0=1; bool is_part=true;//FIXME
  int part_n=int(this->mesh_part.size()) - part_0;
  //
  int dots_mod=0;
  if(verbosity==2){ dots_mod=1;
    if( part_n>(comp_n*4) ){ dots_mod=part_n; }
  }
  //
  INT_MESH elem_tot=0, halo_elem_tot=0;
  INT_MESH node_tot=0, halo_node_tot=0,halo_remo_tot=0,halo_loca_tot=0;
  INT_MESH halo_udof_tot=0;
  std::string solv_name="";
  INT_MESH halo_n=0;
#pragma omp parallel num_threads(comp_n)
{
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
  // Read Mesh ======================================================
    std::stringstream ss;
    ss << this->base_name; if(is_part){ ss << "_" << part_i << ".fmr" ;}
    std::string pname = ss.str();
#if VERB_MAX>3
    if(verbosity>3){
    //if(part_n<10){
      std::cout << "Reading " << pname << "..." <<'\n';
    //};
    }
#endif
    Mesh::part t;
    this->ReadPartFMR(t,pname.c_str(),false);
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=t;
    Y->solv_cond = this->solv_cond;
    if(dots_mod>0){
      if((part_i%dots_mod)==0){ std::cout <<"."; fflush(stdout); } }
#if VERB_MAX>10
    if(verbosity>10){
#pragma omp critical(print)
{
    uint d=uint(Y->node_d);
    for(INT_MESH i=0; i<E->node_n; i++){
      std::cout << E->node_glid[i] <<":";
      for(uint j=0; j<d; j++){
        std::cout <<" "<< S->part_d[d* i+j];
      } std::cout <<'\n';
    }
}
    }
#endif
#pragma omp critical(systot)
{
    //this->meth_name=S->meth_name;
    this->mesh_part[part_i] = t;
    //M->list_elem[part_i] = E;
    this->elem_n += E->elem_n;
    this->node_n += E->node_n - E->halo_remo_n;
    this->udof_n += long(Y->node_d) * long(E->node_n - E->halo_remo_n);
    this->solv_flop += float(S->udof_flop *uint(Y->node_d) * (E->node_n - E->halo_remo_n));
    this->solv_band += float(S->udof_band *uint(Y->node_d) * (E->node_n - E->halo_remo_n));
    this->phys_flop += float(Y->tens_flop);
    this->phys_band += float(Y->tens_band);
}
  }// End parallel read & setup loop ====================================
#if OMP_NESTED==true
  std::vector<part> priv_part;
  priv_part.resize(this->mesh_part.size());
  std::copy(this->mesh_part.begin(), this->mesh_part.end(), priv_part.begin());
#else
  // Make thread-local copies of mesh_part into threadprivate priv_part.
#pragma omp critical
{
  priv_part.resize(this->mesh_part.size());
  std::copy(this->mesh_part.begin(), this->mesh_part.end(), priv_part.begin());
}
#endif
    //----------------------------------------------------------- Sync halo_map
#pragma omp for schedule(static)
    for(int part_i=part_0; part_i<(part_n+part_0); part_i++){
      Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=priv_part[part_i];
#pragma omp critical(halomap)
{//FIXME critical section here?
      if(E->node_haid.size()==0){ E->node_haid.resize(E->halo_node_n); };
      for(INT_MESH i=0; i<E->halo_node_n; i++){
        INT_MESH g=E->node_glid[i];
        if(this->halo_map.count(g)==0){// Initialize halo_map
          this->halo_map[g]=halo_n;
          E->node_haid[i]=halo_n;
          halo_n++;
        }else{// Add in the rest.
          E->node_haid[i]=this->halo_map[g];
        }
      }
}
    }// End halo map init
}// End parallel region ==================================================
  //this->udof_n = part_udof_n;
  //read_done = std::chrono::high_resolution_clock::now();
  if(dots_mod>0){ std::cout <<'\n'; };
  //for(uint i=0; i<M->list_elem.size(); i++){
  for(uint i=0; i<this->mesh_part.size(); i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=this->mesh_part[i];
#if 0
    // Sync global bounding box.
    for(int i=0; i<6; i++){ E->glob_bbox[i]=this->glob_bbox[i];
      //printf(" %f",E->glob_bbox[i]);
    }
#endif
    //std::cout << "Partition " << i ;
    if(verbosity>2){ printf("Partition %u", i); }
    if( E == NULL ){
      if(verbosity>2){ printf(" is null.\n"); }
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
      }
#endif
      if(Y->mtrl_prop.size()>2){
        printf("Orthotropic Elastic: E=%9.2e, nu=%4.2f, G:%9.2e\n",
          Y->mtrl_prop[0], Y->mtrl_prop[1], Y->mtrl_prop[2] );
      }else{
        printf("  Isotropic Elastic: E=%9.2e, nu=%4.2f, G:iso\n",
          Y->mtrl_prop[0], Y->mtrl_prop[1] );
      }
      if(Y->ther_expa.size()>0){
        printf("  Thermal Expansion:");
        for(uint i=0; i<Y->ther_expa.size(); i++){
          printf("%9.2e",Y->ther_expa[i]); }
        printf("\n");
      }
      if(Y->ther_cond.size()>0){
        printf("       Conductivity:");
        for(uint i=0; i<Y->ther_cond.size(); i++){
          printf("%9.2e",Y->ther_cond[i]); }
        printf("\n");
      }
      if(Y->plas_prop.size()>0){
        printf("         Plasticity:");
        for(uint i=0; i<Y->plas_prop.size(); i++){
          printf("%9.2e",Y->plas_prop[i]); }
        printf("\n");
      }
      printf("   Tensor Constants:");
      for(uint i=0; i<Y->mtrl_matc.size(); i++){
        printf("%9.2e",Y->mtrl_matc[i]);
        if( ((i+0)%3)==2){ printf("\n");
        if( i<(Y->mtrl_matc.size()-1) ){ printf("                    "); }
        }
      }
      if(Y->mtrl_dirs.size()>2){
      printf("            Rotated: [%+6.2f,%+6.2f,%+6.2f] rad around [z,x,z].\n",
        Y->mtrl_dirs[0],Y->mtrl_dirs[1],Y->mtrl_dirs[2] ); }
      //std::cout <<"  Preconditioner: ";
      //for(size_t i=0;i<S->part_d.size();i++){ std::cout << S->part_d[i] <<" ";};
      //std::cout <<'\n';
    }
#endif
    solv_name      = S->meth_name;
    elem_tot      += E->elem_n;
    halo_elem_tot += E->halo_elem_n;
    node_tot      += E->node_n;
    halo_node_tot += E->halo_node_n;
    halo_remo_tot += E->halo_remo_n;
    halo_loca_tot += E->halo_loca_n;
    halo_udof_tot += E->halo_node_n * Y->node_d;
    }
  }//end serial partition loop
  this->halo_val.resize(halo_udof_tot);//node_d*halo_loca_tot);
#if VERB_MAX > 1
      if(verbosity>1){
        printf(" Parts:");
        if(iso3_part_n>0){ printf("%10u iso",iso3_part_n); }
        if(ort3_part_n>0){ printf("%10u ortho",ort3_part_n); }
        if(ther_part_n>0){ printf(" (%u thermo)",ther_part_n); }
        if(plas_part_n>0){ printf(" (%u plastic)",plas_part_n); }
        printf("\n");
      }
#endif
#if VERB_MAX>1
  if(verbosity>1){
    printf(" Total:%10u Elems (%u halo) in %u partitions\n",
      elem_tot, halo_elem_tot, part_n);
    printf("       %10u Nodes (%u halo: %u remote, %u local)\n",
          node_tot, halo_node_tot, halo_remo_tot, halo_loca_tot);
#if 0
    //printf("Read and set up in %f s.\n", float(read_time.count())*1e-9 );
    //std::cout <<"Solving ";
    //std::cout <<"system: "<<this->_elem_n<<" Elems, "
    //  <<this->node_n<<" Nodes, "<<this->udof_n<<" DOF; "<<'\n'
    //  <<"     to within: "<<rtol<<" relative tolerance,"<<'\n'
    //  <<"or stopping at: "<<iter_max<<" "<<solv_name<<" iterations..."<<'\n';
#endif
  }
#endif
  return 0;
}
int Mesh::ReadPartFMR( part& P, const char* fname, bool is_bin ){
  Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P;
  INT_ORDER pord=1;
  E = new Tet(pord);//FIXME
  E->simd_n = this->simd_n;//FIXME
  //
  std::string s; if(is_bin){ s="binary";}else{s="ASCII";};
  if(is_bin){
    std::cout << "ERROR Could not open "<< fname << " for reading." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return 0;
  }
  //FLOAT_MESH bbox[6]={9e9,9e9,9e9 , -9e9,-9e9,-9e9};// global bounding box
  //FLOAT_MESH minx=9e9, miny=9e9, minz=9e9, maxx=-9e9,maxy=-9e9,maxz=-9e9;
  std::string fmrstring;
  std::ifstream fmrfile(fname);//return 0;
  Phys::vals t_mtrl_prop={},t_mtrl_dirs={}, t_ther_cond={},t_ther_expa={},
    t_plas_prop={};
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
      E->elem_d=(INT_DOF)d;
      E->elem_p =(INT_ORDER)p;
    };
    if(fmrstring=="$Conn"){
      int c;
      fmrfile >> c >> E->elem_n >> E->halo_elem_n ;
      E->elem_conn_n =(INT_ELEM_NODE)c;
#if 0
      // Oversize elem_conn for prefetching...doesn't help
      E->elem_conn.resize(uint(E->elem_conn_n)*(E->elem_n+1));
      for(int i=0;i< E->elem_conn_n;i++){//FIXME fill with last entry
        E->elem_conn[E->elem_conn_n*E->elem_n +i ]=0 ;}
#else
      E->elem_conn.resize(uint(E->elem_conn_n)*E->elem_n);
#endif
      E->elem_glid.resize(E->elem_n);
      for(uint i=0; i<E->elem_n; i++){
        fmrfile >> E->elem_glid[i];
        for(uint j=0; j<uint(c);j++){ fmrfile >> E->elem_conn[uint(c)* i+j]; };
      };
    };
    if(fmrstring=="$Node"){
      int d,v;
      fmrfile >> d >> v >> E->node_n
        >> E->halo_node_n >> E->halo_remo_n >> E->halo_loca_n ;
      E->mesh_d=(INT_ORDER)d; E->vert_n=v;
    };
#if 0
    if(fmrstring=="$HaloNodeID"){
      node_glid.resize(halo_node_n);
      for(size_t i=0; i<node_glid.size(); i++){
        fmrfile >> node_glid[i]; 
        node_loid[node_glid[i]] = i;
      };
    };
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
    };
#endif
    if(fmrstring=="$VertCoor"){
      E->node_coor.resize(uint(E->mesh_d)*E->node_n);//FIXME vert_n
      E->node_glid.resize(E->node_n);
      FLOAT_MESH x=0.0;
      for(INT_MESH i=0; i<E->node_n; i++){//FIXME vert_n
        fmrfile >> E->node_glid[i];
        E->node_loid[E->node_glid[i]] = i;
        for(uint j=0; j<(uint)E->mesh_d; j++){
          fmrfile >> x;
          E->node_coor[E->mesh_d *i+j]=x;
          if(x<E->loca_bbox[  j]){E->loca_bbox[  j]=x;}
          if(x>E->loca_bbox[3+j]){E->loca_bbox[3+j]=x;}
        }
      }
#if 1
      //printf("Bounds: ");
      for(uint i=0; i<3; i++){
#pragma omp critical
{
          if(E->loca_bbox[i]<this->glob_bbox[i]){
            this->glob_bbox[i]=E->loca_bbox[i];}
          if(E->loca_bbox[3+i]>this->glob_bbox[3+i]){
            this->glob_bbox[3+i]=E->loca_bbox[3+i];}
}
      }
        //E->loca_bbox[i]=bbox[i]; }
      //  printf(" %f",bbox[i]);
      //}
      //printf("\n");
#endif
    }
    if(fmrstring=="$BC0"){ INT_MESH n,f,m;
      fmrfile >> m;
      for(INT_MESH i=0; i<m; i++){
        fmrfile >> n >> f ; 
        E->bc0_nf.insert(Mesh::nfitem(n,f));
      };
    };
    if(fmrstring=="$BCS"){ INT_MESH n,f,m; FLOAT_SOLV v;
      fmrfile >> m;
      for(INT_MESH i=0; i<m; i++){
        fmrfile >> n >> f >> v; 
        E->bcs_vals.insert(Mesh::nfval(n,f,v));
      };
    };
    if(fmrstring=="$RHS"){ INT_MESH n,f,m; FLOAT_SOLV v;
      fmrfile >> m;
      for(INT_MESH i=0; i<m; i++){
        fmrfile >> n >> f >> v; 
        E->rhs_vals.insert(Mesh::nfval(n,f,v));
      };
    };
    if(fmrstring=="$ElasticProperties"){//FIXME Deprecated
      if(t_mtrl_prop.size()==0){
        int s=0; fmrfile >> s;
        t_mtrl_prop.resize(s);
        for(int i=0; i<s; i++){ fmrfile >> t_mtrl_prop[i]; }
        s=0; fmrfile >> s;
        if(s>0){
          t_mtrl_dirs.resize(s);
        for(int i=0; i<s; i++){ fmrfile >> t_mtrl_dirs[i]; }
        }
      }
    }
    if(fmrstring=="$Orientation"){
      int s=0; fmrfile >> s;
      if(s>0){
        t_mtrl_dirs.resize(s);
        for(int i=0; i<s; i++){ fmrfile >> t_mtrl_dirs[i]; }
      }
    }
    if(fmrstring=="$Elastic"){
      int s=0; fmrfile >> s;
      if(s>0){
        t_mtrl_prop.resize(s);
        for(int i=0; i<s; i++){ fmrfile >> t_mtrl_prop[i]; }
      }
    }
    if(fmrstring=="$ThermalExpansion"){
      int s=0; fmrfile >> s;
      t_ther_expa.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> t_ther_expa[i]; }
    }
    if(fmrstring=="$ThermalConductivity"){
      int s=0; fmrfile >> s;
      t_ther_cond.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> t_ther_cond[i]; }
    }
    if(fmrstring=="$Plastic"){// Plasticity Constants
      int s=0; fmrfile >> s;
      t_plas_prop.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> t_plas_prop[i]; }
    }
    //FIXME Add ThermalElastic section?
#if 0
    //if(fmrstring=="$Physics"){ //FIXME
    if(fmrstring=="$ElasticIsotropic"){ //FIXME
      fmrfile >> m;// Should be 2
      //for(INT_MESH i=0; i<m; i++){
         fmrfile >> young >> poiss ; 
      //};
    };
    if(fmrstring=="$ElasticOrthotropic"){ //FIXME
    };
    //std::cout <<"*" << fmrstring <<"*" ;
#endif
  }//EOF
  bool has_therm=false, has_plas=false;
  if( (t_ther_expa.size()>0) | (t_ther_cond.size()>0) ){ has_therm=true; }
  if(  t_plas_prop.size()>0 ){ has_plas=true; }
  if( has_therm ){
    if(t_mtrl_dirs.size()<3){
      Y = new ThermElastIso3D(t_mtrl_prop,t_mtrl_dirs,t_ther_expa,t_ther_cond);
#if VERB_MAX>1
#pragma omp atomic update
      this->iso3_part_n+=1;
#pragma omp atomic update
      this->ther_part_n+=1;
#endif
    }else{
      Y = new ThermElastOrtho3D(t_mtrl_prop,t_mtrl_dirs,t_ther_expa,t_ther_cond);
#if VERB_MAX>1
#pragma omp atomic update
      this->ort3_part_n+=1;
#pragma omp atomic update
      this->ther_part_n+=1;
#endif
    }
  }else if( has_plas ){
    if(t_plas_prop.size() < 3){
      Y = new ElastPlastKHIso3D(t_mtrl_prop[0],t_mtrl_prop[1]);
      Y->plas_prop.resize(t_plas_prop.size());
      for(uint i=0; i<t_plas_prop.size(); i++){
        Y->plas_prop[i] = t_plas_prop[i]; }
#if VERB_MAX>1
#pragma omp atomic update
      this->iso3_part_n+=1;
#pragma omp atomic update
      this->plas_part_n+=1;
#endif
    }//else{
//      Y = new ElastPlastKHOrt3D(t_mtrl_prop,t_mtrl_dirs);
//#if VERB_MAX>1
//#pragma omp atomic update
//      this->ort3_part_n+=1;
//#pragma omp atomic update
//      this->plas_part_n+=1;
//#endif
//    }
  }else{
    if(t_mtrl_dirs.size()<3){
      Y = new ElastIso3D(t_mtrl_prop[0],t_mtrl_prop[1]);
#if VERB_MAX>1
#pragma omp atomic update
      this->iso3_part_n+=1;
#endif
    }else{
      Y = new ElastOrtho3D(t_mtrl_prop,t_mtrl_dirs);
#if VERB_MAX>1
#pragma omp atomic update
      this->ort3_part_n+=1;
#endif
    }
  }
  switch( this->solv_meth ){
    case(Solv::SOLV_CG):{
      S=new PCG( E->node_n * Y->node_d, this->iter_max, glob_rtol ); 
      break;}
    case(Solv::SOLV_CR):{
      S=new PCR( E->node_n * Y->node_d, this->iter_max, glob_rtol ); 
      break;}
    case(Solv::SOLV_NG):{
      S=new NCG( E->node_n * Y->node_d, this->iter_max, glob_rtol ); 
      break;}
    default:{ S=new PCG(E->node_n * Y->node_d, this->iter_max, glob_rtol );  }
  };
  //S->tol = this->glob_rtol;
  S->cube_init=this->cube_init;
  //if(dots_mod>0){
  //  if((part_i%dots_mod)==0){ std::cout <<"."; fflush(stdout); }; };
  E->Setup();
  Y->Setup( E );// Applies material rotations & sets Y->elem_flop and elem_band
  S->Setup( E,Y );// Applies BCs & sets S->udof_flop and
  // For PCR, also computes diagonal of K for Jacobi preconditioner
  //FIXME AND does the initial ElemLinear
  //
  Mesh::part tP(E,Y,S); P=tP;
  //
  return 0;
}
//-------------------------------------------------------------------
int Mesh::SavePartFMR( const part& P, const char* fname, bool is_bin ){
  Elem* E; Phys* Y; Solv* S;
  std::tie(E,Y,S)=P;
  //
  std::string s; if(is_bin){ s="binary"; }else{ s="ASCII"; }
  if(is_bin){
    std::cout << "ERROR Could not save "<< fname << "." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return 1;
  }
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
  fmrfile <<""<< int(E->elem_d) <<" "<< eltype <<" "<< int(E->elem_p) <<'\n';
  fmrfile << "$Conn" <<'\n';
  fmrfile <<""<< int(E->elem_conn_n) <<" "<< E->elem_n <<" "<< E->halo_elem_n <<'\n';
  for(INT_MESH i=0; i<E->elem_n; i++){
    fmrfile << E->elem_glid[i];
    for(uint j=0; j<uint(E->elem_conn_n); j++){
      fmrfile << " " << E->elem_conn[uint(E->elem_conn_n)*i +j];
    }
    fmrfile<<'\n';
  }
  fmrfile << "$Node" <<'\n';
  fmrfile <<""<< int(E->mesh_d) <<" "<< E->vert_n <<" "<< E->node_n
    <<" "<< E->halo_node_n <<" "<< E->halo_remo_n<<" "<< E->halo_loca_n <<'\n';
#if 0
  if(node_glid.size()>0){//E->halo_node_n){
    fmrfile << "$HaloNodeID" ;
    for(size_t i=0; i<node_glid.size(); i++){
      if(!(i%20)){ fmrfile<<'\n'; }
      fmrfile << " " << node_glid[i]; } fmrfile<<'\n';
  }
  if(halo_loca.size()>0){
    fmrfile << "$HaloLocal" ;
    for(size_t i=0; i<halo_loca.size(); i++){
      if(!(i%20)){ fmrfile<<'\n'; }
      fmrfile << " " << halo_loca[i]; } fmrfile<<'\n';
  }
#endif
  fmrfile << "$VertCoor"<<'\n';;
  for(INT_MESH i=0; i<E->vert_n; i++){
    fmrfile << E->node_glid[i];
    for(uint j=0; j<(uint)E->mesh_d; j++){
      fmrfile <<" "<< E->node_coor[E->mesh_d* i+j]; }
    fmrfile <<'\n';
  }
  if(E->bc0_nf.size()>0){
    fmrfile << "$BC0" <<'\n'<<""<< E->bc0_nf.size() <<'\n';
    for(auto i : E->bc0_nf ){ int n,f; std::tie(n,f)=i;
      fmrfile << "" << n <<" "<< f <<'\n'; }
  }
  if(E->bcs_vals.size()>0){
    fmrfile << "$BCS" <<'\n'<<""<< E->bcs_vals.size() <<'\n';
    for(auto i : E->bcs_vals ){ int n,f; FLOAT_MESH v; std::tie(n,f,v)=i;
      fmrfile << "" << n <<" "<< f <<" "<< v <<'\n'; }
  }
  if(E->rhs_vals.size()>0){
    fmrfile << "$RHS" <<'\n'<<""<< E->rhs_vals.size() <<'\n';
    for(auto i : E->rhs_vals ){ int n,f; FLOAT_MESH v; std::tie(n,f,v)=i;
      fmrfile << "" << n <<" "<< f <<" "<< v <<'\n'; }
  }
  fmrfile.close();
  Y->SavePartFMR( fname, is_bin );
  return 0;
}
//-------------------------------------------------------------------
