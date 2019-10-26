#ifndef INCLUDED_GMSH_H
#define INCLUDED_GMSH_H
#include <tuple>
#include <deque>
#include <unordered_map>
#include <vector>
#include "femera.h"

class Gmsh final: public Mesh{
public: Gmsh() : Mesh(Solv::SOLV_CG){
    typeEleNodes[  1]=  2;
    typeEleNodes[  2]=  3;
    typeEleNodes[  3]=  4;
    typeEleNodes[  4]=  4;
    typeEleNodes[  5]=  8;
    typeEleNodes[  6]=  6;
    typeEleNodes[  7]=  5;
    typeEleNodes[  8]=  3;
    typeEleNodes[  9]=  6;
    typeEleNodes[ 10]=  9;
    typeEleNodes[ 11]= 10;
    typeEleNodes[ 12]= 27;
    typeEleNodes[ 13]= 18;
    typeEleNodes[ 14]= 14;
    typeEleNodes[ 15]=  1;
    typeEleNodes[ 16]=  8;
    typeEleNodes[ 17]= 20;
    typeEleNodes[ 18]= 15;
    typeEleNodes[ 19]= 13;
    typeEleNodes[ 20]=  9;
    typeEleNodes[ 21]= 10;
    typeEleNodes[ 22]= 12;
    typeEleNodes[ 23]= 15;
    typeEleNodes[ 24]= 15;
    typeEleNodes[ 25]= 21;
    typeEleNodes[ 26]=  4;
    typeEleNodes[ 27]=  5;
    typeEleNodes[ 28]=  6;
    typeEleNodes[ 29]= 20;
    typeEleNodes[ 30]= 35;
    typeEleNodes[ 31]= 56;
    typeEleNodes[ 92]= 64;
    typeEleNodes[ 93]=125;
    typeEleNodes[137]= 16;
  }//FIXME solv method not needed
  typedef std::tuple<int,INT_DOF,FLOAT_SOLV> tfval;
  typedef std::deque<tfval> tfvals;// for RHS [and fixed-nonzero BCs]
  typedef std::tuple<int,INT_DOF> tfitem;
  typedef std::deque<tfitem> tflist;// for fixed-zero boundary conditions
  //
  bool calc_band=false;//FIXME estimate average matrix nonzero bandwidth
  //
  // Apply to nodes in surfaces with gmsh physical id tags given
  tfvals rhs_tvals={};// Nodal forces applied (nonzeros) to tags
  tfvals bcs_tvals={};// Dirichlet bundary conditions (nonzeros) applied to tags
  tflist bc0_tnf  ={};// Essential (u=0) Dirichlet BCs (node_id,dof_id) tuples
  //
  // Apply to nodes with gmsh global ids given
  tfvals rhs_nvals={};// Nodal forces applied (nonzeros) to node global ids
  tfvals bcs_nvals={};// Dirichlet bundary conditions (nonzeros) applied
  tflist bc0_nnf  ={};// Essential (u=0) Dirichlet BCs (node_id,dof_id) tuples
  //
  //Elem* ReadFile( const char*, INT_ORDER pord );
  Elem* ReadMsh2( const char* );
  Elem* ReadMsh4( const char* );//FIXME Remove pord arg
  //
  std::vector<std::string> file_type_name={"ASCII","binary"};
  // Number of nodes of Gmsh typeEle elem_node_n=typeEleNodes[typeEle];
  std::unordered_map<int,INT_MESH> typeEleNodes;
  //const RESTRICT Mesh::ints typeEleNodes={ 0,2,3,4,4,8,6,5,// first-order elems
  //  3,6,9,10,27,18,14,// second-order elems
  //  1,// point
  //  8,20,15,13,// more second order
  //  9,10,12,15,15,21,4,5,6,20,35,56 };
  //
  // elms_phid[physical_id]={el1,el2,...};
  std::unordered_map<int,std::vector<int>> elms_phid;
  std::unordered_map<int,std::vector<int>> elms_slid;// slice ids
  ints part_slic={1,1,1};
  /*
  typeEleNodes[ 1]= 2;// Linear Bar
  typeEleNodes[ 2]= 3;// Linear Triangle
  typeEleNodes[ 3]= 4;// Linear Quad
  typeEleNodes[ 4]= 4;// Linear Tet
  typeEleNodes[ 5]= 8;// Linear Brick
  typeEleNodes[ 6]= 6;// Linear Prism
  typeEleNodes[ 7]= 5;// Linear Pyramid
  //
  typeEleNodes[15]= 1;// Point
  //
  //NOTE Second-order
  // Lagrange elements not supported
  typeEleNodes[ 8]= 3;// Quadratic Bar
  typeEleNodes[ 9]= 6;// Quadratic Triangle
  typeEleNodes[10]= 9;// Quadratic Quad
  typeEleNodes[11]=10;// Quadratic Tet
  typeEleNodes[12]=27;// Quadratic Brick
  typeEleNodes[13]=18;// Quadratic Prism
  typeEleNodes[14]=14;// Quadratic Pyramid
  //
  // Serendipity elements
  typeEleNodes[16]= 8;// Quadratic Quad
  typeEleNodes[17]=20;// Quadratic Brick
  typeEleNodes[18]=15;// Quadratic Prism
  typeEleNodes[19]=13;// Quadratic Pyramid
  //NOTE Third and higher order not supported
  */
  int Init() final;
  int Iter() final;
protected:
  //std::deque(Elem*) list_elem;
private:
};
/*
Elem* Gmsh::ReadFile( const char* fname, INT_ORDER pord ){
  Elem* E;
  if(true){ E=this->ReadMsh2( fname, pord ); }
  else    { E=this->ReadMsh4( fname, pord ); };
  return E;
};*/
#endif
