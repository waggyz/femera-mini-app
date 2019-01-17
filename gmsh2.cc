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
int Gmsh::Init(){ return 1; };
int Gmsh::Iter(){ return 1; };
Elem* Gmsh::ReadMsh2( const char* fname ){
  INT_ORDER pord=1;
  //std::vector<Elem*> elist;
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
  RESTRICT Mesh::vals vert_coor ={}; int mesh_d=3;
  RESTRICT Mesh::ints node_glid ={};// xref from local to global node number
  std::vector<INT_MESH> elem_glid ={};// xref from local to global element number
  std::unordered_map<int,INT_MESH> node_loid;
  std::unordered_map<int,INT_MESH> elem_loid;
  //
  std::unordered_map<int, std::set<int>>
    line_nodes_tagged, surf_nodes_tagged;//, volu_nodes_tagged,
  //  volu_elems_tagged, volu_elems_parted;
  //
  std::ifstream mshfile(fname);
  while( mshfile >> mshstring ){//===================================
    if(mshstring=="$MeshFormat"){//----------------------------------
      mshfile >> version >> file_type >> data_size ;
#if VERB_MAX>2
      std::cout << "Reading Gmsh " << version << " "
        << file_type_name[file_type] << " file "<< fname <<"..." <<'\n';
#endif
      if(file_type==1){
      std::cout << "WARNING Gmsh binary file format not supported.";
      mshfile >> onendian; };// Detect binary file endianness
    };// Done reading header
    if(mshstring=="$Nodes"){//---------------------------------------
      mshfile >> number_of_nodes ; node_n=(INT_MESH)number_of_nodes;
      vert_coor.resize(node_n*mesh_d);
      node_glid.resize(node_n);
#if VERB_MAX>3
      std::cout << "Reading " << number_of_nodes << " nodes..." <<'\n';
#endif
      for(int i=0; i<number_of_nodes; i++){
        mshfile >> node_number >> x>>y>>z ;
#if VERB_MAX>3
        std::cout << node_number <<": " <<x<<","<<y<<","<<x <<'\n';
#endif
        node_glid[i]=(INT_MESH)node_number;
        node_loid[node_number]=i;
        vert_coor[mesh_d* i+0]=x;
        vert_coor[mesh_d* i+1]=y;
        vert_coor[mesh_d* i+2]=z;
      };
    };// Done reading nodes
    if(mshstring=="$Elements"){//------------------------------------
      bool is_volu=false,is_line=false,is_surf=false;
      INT_MESH elxx_i=0;
      mshfile >> number_of_elements ;
#if VERB_MAX>3
      std::cout << "Reading " << number_of_elements << " elements..." <<'\n';
#endif
      for(int i=0; i<number_of_elements; i++){
        mshfile >> elm_number >> elm_type >> number_of_tags ;
        is_volu=false; is_line=false; is_surf=false;
        switch(elm_type){
          case( 4):{pord=1; is_volu=true; elxx_i++;
            elem_loid[elm_number]=elem_glid.size();
            elem_glid.push_back(elm_number);
            break;}
          case( 2):{pord=1; is_line=true; break;}
          case( 3):{pord=1; is_surf=true; break;}
          // Quadratic elements
          case(11):{pord=2; is_volu=true; elxx_i++;
            elem_loid[elm_number]=elem_glid.size();
            elem_glid.push_back(elm_number);
            break;}
          case( 8):{pord=2; is_line=true; break;}
          case( 9):{pord=2; is_surf=true; break;}
          // Cubic elements
          case(29):{pord=3; is_volu=true; elxx_i++;
            elem_loid[elm_number]=elem_glid.size();
            elem_glid.push_back(elm_number);
            break;}
          case(26):{pord=3; is_line=true; break;}
          case(20):{pord=3; is_surf=true; break;}
          case(21):{pord=3; is_surf=true; break;}
          //case(137):{pord=3; is_volu=true; elxx_i++;
          //  elem_loid[elm_number]=elem_glid.size();
          //  elem_glid.push_back(elm_number);
          //  break;}
          default:{}
        }
        for(int j=0; j<number_of_tags; j++){
          mshfile >> tag;
          if(j==0){ physical_tag = tag;
            if(is_volu){ this->elms_phid[tag].push_back(elm_number); };
          };//FIXME Is this always true?
        };
#if VERB_MAX>3
        std::cout << elm_number <<" ["<< physical_tag <<"]: ";
#endif
        for(uint j=0; j<this->typeEleNodes[elm_type]; j++){
          mshfile >> node_number;
#if VERB_MAX>3
        std::cout << node_number <<" ";
#endif
          if(is_volu){tet_conn.push_back((INT_MESH)node_number); };
          if(is_line){line_nodes_tagged[physical_tag].insert(node_number); };
          if(is_surf){surf_nodes_tagged[physical_tag].insert(node_number); };
        };
#if VERB_MAX>3
        std::cout << '\n';
#endif
      };
      elxx_n=elxx_i;
#if VERB_MAX>3
      std::cout << "Found " << elxx_n << " Tet Elements." <<'\n';
#endif
      if(elxx_n>0){
        E = new Tet(pord,elxx_n);
        //std::copy(&tet_conn[0   ],
        //          &tet_conn[elxx_n*E->elem_conn_n],
        //          &E->elem_conn[0] );
        for(INT_MESH i=0; i<tet_conn.size(); i++){
          E->elem_conn[i]=node_loid[tet_conn[i]];
        };
        //
        E->mesh_d=mesh_d;
        E->node_n=node_n;
        E->vert_n=node_n;
        E->vert_coor.resize(vert_coor.size());
        E->vert_coor=vert_coor;
        //E->vert_coor.resize(vert_coor.size());//FIXED Try simple assignment?
        //std::copy(&vert_coor[0   ],
        //          &vert_coor[mesh_d*node_n],
        //          &E->vert_coor[0] );
        E->node_glid.resize(node_glid.size());
        E->node_glid=node_glid;
        E->node_loid=node_loid;
        E->elem_loid=elem_loid;
        E->elem_glid.resize(elem_glid.size());
        std::copy(&elem_glid[0   ],
                  &elem_glid[elem_glid.size()],
                  &E->elem_glid[0] );
      };
    };// Done reading elements
#if VERB_MAX>3
    // Not supported ------------------------------------------------
    if(mshstring=="$PhyscialNames"){std::cout << mshstring <<'\n';};
    if(mshstring=="$Periodic"){std::cout << mshstring <<'\n';};
    if(mshstring=="$NodeData"){std::cout << mshstring <<'\n';};
    if(mshstring=="$ElementData"){std::cout << mshstring <<'\n';};
    if(mshstring=="$ElementNodeData"){std::cout << mshstring <<'\n';};
    if(mshstring=="$InterpolationScheme"){std::cout << mshstring <<'\n';};
    //
    if(mshstring=="$EndMeshFormat"){std::cout << mshstring <<'\n';};
    if(mshstring=="$EndPhysicalNames"){std::cout << mshstring <<'\n';};
    if(mshstring=="$EndNodes"){std::cout << mshstring <<'\n';};
    if(mshstring=="$EndElements"){std::cout << mshstring <<'\n';};
    if(mshstring=="$EndPeriodic"){std::cout << mshstring <<'\n';};
    if(mshstring=="$EndNodeData"){std::cout << mshstring <<'\n';};
    if(mshstring=="$EndElementData"){std::cout << mshstring <<'\n';};
    if(mshstring=="$EndElementNodeData"){std::cout << mshstring <<'\n';};
    if(mshstring=="$EndInterpolationScheme"){std::cout << mshstring <<'\n';};
#endif
  };// EOF ==========================================================
  {// Scope these local variables
  int n,t; INT_DOF f; FLOAT_SOLV v;
  //
  // Specified nodes
  for( auto r : this->rhs_nvals){// Specific nodes getting nodal forces
    std::tie(n,f,v) = r;
    E->rhs_vals.insert(nfval(E->node_loid[n],f,v));
  };
  for( auto r : this->bcs_nvals){// Specific nodes getting fixed u
    std::tie(n,f,v) = r;
    E->bcs_vals.insert(nfval(E->node_loid[n],f,v));
  };
  for( auto r : this->bc0_nnf){// Specific nodes getting u=0
    std::tie(n,f) = r;
    //E->bc0_nf.push_back(nfitem(node_glob2loca[n],f));
    E->bc0_nf.insert(nfitem(E->node_loid[n],f));
  };
  // Nodes in tagged surfaces/lines (//FIXME check if lines break something
  for( auto r : this->rhs_tvals){// Tagged nodes getting nodal forces
    std::tie(t,f,v) = r;
    for(auto tn : surf_nodes_tagged[t]){
      E->rhs_vals.insert(nfval(E->node_loid[tn],f,v));
      //std::cout << "tag: " << t << " node: " << n << " dof: " << uint(f)
      //  << " val: " << v <<'\n';
    };
    for(auto tn : line_nodes_tagged[t]){
      E->rhs_vals.insert(nfval(E->node_loid[tn],f,v));
    };
  };
  for( auto r : this->bcs_tvals){// Tagged nodes getting fixed u
    std::tie(t,f,v) = r;
    for(auto tn : surf_nodes_tagged[t]){
      E->bcs_vals.insert(nfval(E->node_loid[tn],f,v));
    };
    for(auto tn : line_nodes_tagged[t]){
      E->bcs_vals.insert(nfval(E->node_loid[tn],f,v));
    };
  };
  for( auto r : this->bc0_tnf){// Tagged nodes getting fixed u
    std::tie(t,f) = r;// std::cout <<"*** I AM HERE ***";
    for(auto tn : surf_nodes_tagged[t]){// std::cout <<"*** and ***";
      //std::cout << tn <<":"<< E->node_loid[tn] <<" ";
      //E->bc0_nf.push_back(nfitem(tn,f));
      E->bc0_nf.insert(nfitem(E->node_loid[tn],f));
    };
    for(auto tn : line_nodes_tagged[t]){
      E->bc0_nf.insert(nfitem(E->node_loid[tn],f));
    };
  };
  }//end scope ======================================================
  //
  return E;
};
