#if VERB_MAX > 0
#include <stdio.h>
#endif
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <array>
#include <utility>//std::pair
#include <set>// This is ordered
#include <vector>
#include "gmsh.h"
int Gmsh::Init(){ return 1; }
int Gmsh::Iter(){ return 1; }
Elem* Gmsh::ReadMsh2( const char* fname ){
  INT_ORDER pord=1;
  Elem* E=new Tet(1);// This should be re-newed before being returned.
  //
  // Parse the file into these variables.
  std::string mshstring, dummy ;
  int file_type, data_size, onendian,
    number_of_nodes, node_number,
    number_of_elements, elm_number, elm_type, number_of_tags, tag ;
  double version, x,y,z;
  //
  // Some working variables.
  int physical_tag=0;
  INT_MESH elxx_n=0;
  std::vector<INT_MESH> tet_conn  ={};
  RESTRICT Mesh::vals node_coor ={}; int mesh_d=3;
  RESTRICT Mesh::ints node_glid ={};// xref from local to global node number
  std::vector<INT_MESH> elem_glid ={};// xref from local to global element number
  std::unordered_map<int,INT_MESH> node_loid;
  std::unordered_map<int,INT_MESH> elem_loid;
  //
  std::unordered_map<int, std::set<int>>
    line_nodes_tagged, surf_nodes_tagged;
  //
  std::ifstream mshfile(fname);
  while( mshfile >> mshstring ){//===================================
    if(mshstring=="$MeshFormat"){//----------------------------------
      mshfile >> version >> file_type >> data_size ;
#if VERB_MAX>2
      if(verbosity>2){
        std::cout << "Reading Gmsh " << version << " "
          << file_type_name[file_type] << " file "<< fname <<"..." <<'\n';
      }
#endif
      if(file_type==1){
      std::cout << "WARNING Gmsh binary file format not supported.";
      mshfile >> onendian; }// Detect binary file endianness
    }// Done reading header
    if(mshstring=="$Nodes"){//---------------------------------------
      mshfile >> number_of_nodes ; node_n=(INT_MESH)number_of_nodes;
      node_coor.resize(node_n*mesh_d);
      node_glid.resize(node_n);
#if VERB_MAX>3
      if(verbosity>3){
      std::cout << "Reading " << number_of_nodes << " nodes..." <<'\n'; }
#endif
      for(int i=0; i<number_of_nodes; i++){
        mshfile >> node_number >> x>>y>>z ;
#if VERB_MAX>3
        std::cout << node_number <<": " <<x<<","<<y<<","<<x <<'\n';
#endif
        node_glid[i]=(INT_MESH)node_number;
        node_loid[node_number]=i;
        node_coor[mesh_d* i+0]=x;
        node_coor[mesh_d* i+1]=y;
        node_coor[mesh_d* i+2]=z;
      }
    }// Done reading nodes
    if(mshstring=="$Elements"){//------------------------------------
      bool is_volu=false,is_line=false,is_surf=false;
      INT_MESH elxx_i=0;
      const INT_PART slic_n
        = this->part_slic[0]*this->part_slic[1]*this->part_slic[2];
      mshfile >> number_of_elements;
#if VERB_MAX>3
      if(verbosity>3){
      std::cout << "Reading " << number_of_elements << " elements..." <<'\n'; }
#endif
      //FIXME row-col index of node nonzeros
      std::unordered_map<int,std::set<INT_MESH>> conn_sets_glid;//FIXME
      for(int i=0; i<number_of_elements; i++){
        mshfile >> elm_number >> elm_type >> number_of_tags;
        is_volu=false; is_line=false; is_surf=false;
        switch(elm_type){
          // Limear elements
          case( 4):{ pord=1; is_volu=true; elxx_i++;
            elem_loid[ elm_number ] = elem_glid.size();
            elem_glid.push_back( elm_number );
            break; }
          case( 2):{ pord=1; is_line=true; break; }
          case( 3):{ pord=1; is_surf=true; break; }
          // Quadratic elements
          case(11):{ pord=2; is_volu=true; elxx_i++;
            elem_loid[ elm_number ] = elem_glid.size();
            elem_glid.push_back( elm_number );
            break; }
          case( 8):{ pord=2; is_line=true; break; }
          case( 9):{ pord=2; is_surf=true; break; }
          // Cubic elements
          case(29):{ pord=3; is_volu=true; elxx_i++;
            elem_loid[ elm_number ] = elem_glid.size();
            elem_glid.push_back( elm_number );
            break; }
          case(26):{ pord=3; is_line=true; break; }
          case(20):{ pord=3; is_surf=true; break ;}
          case(21):{ pord=3; is_surf=true; break; }
#if 0
          case(137):{ pord=3; is_volu=true; elxx_i++;
            elem_loid[elm_number]=elem_glid.size();
            elem_glid.push_back(elm_number);
            break; }
#endif
          default:{}
        }
        for(int j=0; j<number_of_tags; j++){
          mshfile >> tag;
          if(is_volu){
            // Try to save memory by not setting elms_phid when slicing.
            if(number_of_tags>3){
              if(j==3){//FIXME This is the partition number
                //FIXME Is this always true?
                physical_tag = tag;
                if(slic_n<2){
                  this->elms_phid[tag].push_back(elm_number); }
              }
            }else if(j==0){
                physical_tag = tag;
                if(slic_n<2){
                  this->elms_phid[tag].push_back(elm_number); }
              }
          }else if(j==0){// not a volume element
            physical_tag = tag;
          }
        }
#if VERB_MAX>3
        if(verbosity>3){
          std::cout << elm_number <<" ["<< physical_tag <<"]: "; }
#endif
        for(uint j=0; j<this->typeEleNodes[elm_type]; j++){
          mshfile >> node_number;
#if VERB_MAX>3
          if(verbosity>3){ std::cout << node_number <<" "; }
#endif
          if(is_line){line_nodes_tagged[physical_tag].insert(node_number); }
          if(is_surf){surf_nodes_tagged[physical_tag].insert(node_number); }
          if(is_volu){tet_conn.push_back((INT_MESH)node_number); }
        }
        if( (slic_n > 1) && is_volu ){
          uint Dm=mesh_d;
          Mesh::vals c(0.0,Dm);// Centroid of element
          const INT_MESH e = tet_conn.size() - this->typeEleNodes[elm_type];
          for(uint i=0;i<4;i++){// Mean of 4 corner nodes
            const INT_MESH n = node_loid[ tet_conn[ e+i ] ];
            for(uint j=0;j<Dm;j++){
              c[j] += node_coor[Dm* n+j ] * 0.25;
            }
          }
          const FLOAT_MESH sx =(FLOAT_MESH)this->part_slic[0];
          const FLOAT_MESH sy =(FLOAT_MESH)this->part_slic[1];
          const FLOAT_MESH sz =(FLOAT_MESH)this->part_slic[2];
          const INT_PART part_i = 1
            + INT_PART( c[0]*sx )//FIXME Assumes xyz bounds are [0,1]
            + INT_PART( c[1]*sy )*this->part_slic[0]
            + INT_PART( c[2]*sz )*this->part_slic[0]*this->part_slic[1];
            this->elms_slid[part_i].push_back(elm_number);
        }
        if(this->calc_band && is_volu){//FIXME build row-col index of node nonzeros
          const INT_MESH e = tet_conn.size() - this->typeEleNodes[elm_type];
          for(uint j=0;j<this->typeEleNodes[elm_type]; j++){
            const INT_MESH nj = tet_conn[e+j];
            for(uint k=0;k<this->typeEleNodes[elm_type]; k++){
              const INT_MESH nk = tet_conn[e+k];
              conn_sets_glid[nj].insert(nk);
              //std::cout << nj <<":"<< nk << "(" << conn_sets_glid.size() << ")" <<'\n';
            } }
        }
#if VERB_MAX>3
      if(verbosity>3){ std::cout << '\n'; }
#endif
      }// end element loop
      elxx_n=elxx_i;
      if(this->calc_band){//FIXME calculate and report average matrix bandwidth
        FLOAT_MESH nzbw=0.0;
        long int n=0,t=0;
        for(auto s : conn_sets_glid){ t+=1; n += s.second.size(); }
        nzbw = FLOAT_MESH(n) / FLOAT_MESH(t);
        std::cout << "Average Node Connectivity: " << (nzbw) << "." <<'\n';
        std::cout << "Average Matrix Bandwidth : " << (3.0*nzbw) << "." <<'\n';
      }
#if VERB_MAX>3
      if(verbosity>3){
      std::cout << "Found " << elxx_n << " Tet Elements." <<'\n'; }
#endif
      if(elxx_n>0){
        E = new Tet(pord,elxx_n);
        for(INT_MESH i=0; i<tet_conn.size(); i++){
          E->elem_conn[i]=node_loid[tet_conn[i]];
        }
        E->mesh_d=mesh_d;
        E->node_n=node_n;
        E->vert_n=node_n;
        E->node_coor.resize(node_coor.size());
        E->node_coor=node_coor;
        E->node_glid.resize(node_glid.size());
        E->node_glid=node_glid;
        E->node_loid=node_loid;
        E->elem_loid=elem_loid;
        E->elem_glid.resize(elem_glid.size());
        std::copy(&elem_glid[0   ],
                  &elem_glid[elem_glid.size()],
                  &E->elem_glid[0] );
      }
    }// Done reading elements
#if VERB_MAX>3
    if(verbosity>3){
    // Not supported ------------------------------------------------
    if(mshstring=="$PhyscialNames"){std::cout << mshstring <<'\n';}
    if(mshstring=="$Periodic"){std::cout << mshstring <<'\n';}
    if(mshstring=="$NodeData"){std::cout << mshstring <<'\n';}
    if(mshstring=="$ElementData"){std::cout << mshstring <<'\n';}
    if(mshstring=="$ElementNodeData"){std::cout << mshstring <<'\n';}
    if(mshstring=="$InterpolationScheme"){std::cout << mshstring <<'\n';}
    //
    if(mshstring=="$EndMeshFormat"){std::cout << mshstring <<'\n';}
    if(mshstring=="$EndPhysicalNames"){std::cout << mshstring <<'\n';}
    if(mshstring=="$EndNodes"){std::cout << mshstring <<'\n';}
    if(mshstring=="$EndElements"){std::cout << mshstring <<'\n';}
    if(mshstring=="$EndPeriodic"){std::cout << mshstring <<'\n';}
    if(mshstring=="$EndNodeData"){std::cout << mshstring <<'\n';}
    if(mshstring=="$EndElementData"){std::cout << mshstring <<'\n';}
    if(mshstring=="$EndElementNodeData"){std::cout << mshstring <<'\n';}
    if(mshstring=="$EndInterpolationScheme"){std::cout << mshstring <<'\n';}
    }
#endif
  }// EOF ==========================================================
  {// Scope these local variables
  int n,t; INT_DOF f; FLOAT_SOLV v;
  //
  // Specified nodes
  for( auto r : this->rhs_nvals){// Specific nodes getting nodal forces
    std::tie(n,f,v) = r;
    E->rhs_vals.insert(nfval(E->node_loid[n],f,v));
  }
  for( auto r : this->bcs_nvals){// Specific nodes getting fixed u
    std::tie(n,f,v) = r;
    E->bcs_vals.insert(nfval(E->node_loid[n],f,v));
  }
  for( auto r : this->bc0_nnf){// Specific nodes getting u=0
    std::tie(n,f) = r;
    E->bc0_nf.insert(nfitem(E->node_loid[n],f));
  }
  // Nodes in tagged surfaces/lines (//FIXME check if lines break something
  for( auto r : this->rhs_tvals){// Tagged nodes getting nodal forces
    std::tie(t,f,v) = r;
    for(auto tn : surf_nodes_tagged[t]){
      E->rhs_vals.insert(nfval(E->node_loid[tn],f,v));
      //std::cout << "tag: " << t << " node: " << n << " dof: " << uint(f)
      //  << " val: " << v <<'\n';
    }
    for(auto tn : line_nodes_tagged[t]){
      E->rhs_vals.insert(nfval(E->node_loid[tn],f,v));
    }
  }
  for( auto r : this->bcs_tvals){// Tagged nodes getting fixed u
    std::tie(t,f,v) = r;
    for(auto tn : surf_nodes_tagged[t]){
      E->bcs_vals.insert(nfval(E->node_loid[tn],f,v));
    }
    for(auto tn : line_nodes_tagged[t]){
      E->bcs_vals.insert(nfval(E->node_loid[tn],f,v));
    }
  }
  for( auto r : this->bc0_tnf){// Tagged nodes getting fixed u
    std::tie(t,f) = r;// std::cout <<"*** I AM HERE ***";
    for(auto tn : surf_nodes_tagged[t]){// std::cout <<"*** and ***";
      //std::cout << tn <<":"<< E->node_loid[tn] <<" ";
      E->bc0_nf.insert(nfitem(E->node_loid[tn],f));
    }
    for(auto tn : line_nodes_tagged[t]){
      E->bc0_nf.insert(nfitem(E->node_loid[tn],f));
    }
  }
  }//end scope ======================================================
  //
  return E;
}
