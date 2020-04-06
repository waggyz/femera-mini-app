#ifndef INCLUDED_ELEM_H
#define INCLUDED_ELEM_H
#include <cstring>// std::memcpy
#include <vector>
#include <set>
#include <unordered_map>
class Elem {
public:
  enum Tess {// ELEM_TIE=11,
    //ELEM_BAR=12, ELEM_TRI=23, ELEM_QUA=24,
    //ELEM_TET=34, ELEM_BRI=38, ELEM_PRI=36, ELEM_PYR=35 };
    ELEM_SCA=0, ELEM_VEC=1, ELEM_BAR=2, ELEM_TRI=3, ELEM_QUA=4,
    ELEM_TET=5, ELEM_PYR=6, ELEM_PRI=7, ELEM_BRI=8 };
  INT_ORDER mesh_d=3;// Mesh dimension
  INT_ORDER elem_d=3;// Element dimension
  INT_ORDER elem_p=1;
  INT_ORDER gaus_p=1; INT_ORDER gaus_n=1;
  INT_ORDER jacs_f=mesh_d*elem_d + 1;// size of jac matrix + det
  INT_ORDER elem_jacs_n=1;//1 for tri & tet, 3? for qua, 4? for bri
  INT_ELEM_NODE elem_vert_n;// Size of one linear element connectivity
  INT_ELEM_NODE elem_edge_n;// Edges of one element
  INT_ELEM_NODE elem_face_n;// Faces of one element containing a node
  INT_ELEM_NODE elem_conn_n;
  INT_MESH elem_n=0, vert_n=0, node_n=0, jacs_n=0,
    halo_node_n=0, halo_loca_n=0, halo_remo_n=0, halo_elem_n=0;
  // Local halo and system sizes.
  // Halo nodes are [0...halo_n-1]; interior nodes are [halo_n...node_n-1].
  uint simd_n=1;
  //
  FLOAT_MESH loca_bbox[6]={ 9e9,9e9,9e9 ,-9e9,-9e9,-9e9 };
  FLOAT_MESH glob_bbox[6]={ 9e9,9e9,9e9 ,-9e9,-9e9,-9e9 };
  //
  Mesh::vals intp_shpf={};
  Mesh::vals intp_shpg={};
  Mesh::ints elem_conn={};// Grouped so halo nodes come first.
  //FIXME Should be conn_node or elem_node?
  //(mesh_d*elem_vert_n*elem_n) used for jac calc
  Mesh::vals elip_jacs={};
  //FIXME Change to elem_jacs, size: jacs_f * jacs_n
  //FIXME Broke 2D physics
  Mesh::vals gaus_weig={};
  //
  Mesh::vals node_coor={};// Local element nodal coordinates.
  // Nodes are grouped:
  // 0           .. (halo_remo_n-1) [Ghost nodes]
  // halo_remo_n .. (halo_node_n-1) [There are halo_loca_n of these.]
  // halo_node_n .. (     node_n-1) [Interior nodes]
  //
  Mesh::ints elem_glid ={};// xref from local to global element number
  //FIXME Should be <int> to match Gmsh tags
  Mesh::ints node_glid ={};// xref from local to global node number
  // global_id=node_glid[local_id];
  Mesh::ints node_haid ={};// xref from local to halo node number
  // M->node_halo_id = E->node_haid[ node_part_id ]
  std::unordered_map<int,INT_MESH> elem_loid;
  std::unordered_map<int,INT_MESH> node_loid;
  std::vector<int> halo_glid ={};// = loca_glid U remo_glid
  std::vector<int> loca_glid ={};
  std::vector<int> remo_glid ={};
  //NOTE Global node numbers are 1-indexed.
  //NOTE Local (partition) node numbers are 0-indexed.
  //
  // Local boundary conditions [were in Mesh]
  Mesh::nfvals rhs_vals={};// Nodal forces applied (nonzeros)
  Mesh::nfvals bcs_vals={};// Dirichlet bundary conditions (nonzeros)
  Mesh::nflist bc0_nf  ={};// Essential (u=0) Dirichlet BCs (node_id,dof_id)
  //
#if 0
  //template <typename F> static inline
  template <typename F> inline
    int part_resp_glob( F f, Phys* Y,
    const INT_MESH e0, const INT_MESH ee,
    FLOAT_SOLV* RESTRICT part_f, const FLOAT_SOLV* RESTRICT part_u );
#endif
  //
  inline FLOAT_MESH Jac1Det( const FLOAT_MESH  );
  inline FLOAT_MESH Jac2Det( const FLOAT_MESH* );
  inline FLOAT_MESH Jac3Det( const FLOAT_MESH* );
  inline FLOAT_MESH Jac1Det( const RESTRICT Mesh::vals&);
  inline FLOAT_MESH Jac2Det( const RESTRICT Mesh::vals&);
  inline FLOAT_MESH Jac3Det( const RESTRICT Mesh::vals&);
  inline int Jac1Inv( FLOAT_MESH , const FLOAT_MESH);
  inline int Jac2Inv( FLOAT_MESH*, const FLOAT_MESH);
  inline int Jac3Inv( FLOAT_MESH*, const FLOAT_MESH);
  inline int Jac1Inv(RESTRICT Mesh::vals&, const FLOAT_MESH);
  inline int Jac2Inv(RESTRICT Mesh::vals&, const FLOAT_MESH);
  inline int Jac3Inv(RESTRICT Mesh::vals&, const FLOAT_MESH);
  inline int Jac2Tnv(RESTRICT Mesh::vals&, const FLOAT_MESH);
  inline int Jac3Tnv(RESTRICT Mesh::vals&, const FLOAT_MESH);
  //
  int Jac1Dets( );
  int Jac2Dets( );
  int Jac3Dets( );
  virtual int JacsDets( )=0;
  //
  //int ScatterVert2Elem( );
  //int ScatterVert2Elem( Mesh* );//FIXME No longer needed?
  //
  int Setup( );
  int Setup( Mesh* );//FIXME No longer needed?
  //
  // Shape Function Bulk Eval -----------------------------
  const Mesh::vals ShapeFunction(const INT_ORDER, RESTRICT Mesh::vals);
  const Mesh::vals ShapeGradient(const INT_ORDER, const RESTRICT Mesh::vals);
  // Shape Function Kernels -------------------------------
  //FIXME virtual functions are difficult to inline...how to fix?
  virtual const Mesh::vals ShapeFunction(const INT_ORDER, const FLOAT_MESH[])=0;
  virtual const Mesh::vals ShapeGradient(const INT_ORDER, const FLOAT_MESH[])=0;
  // Integration Kernel -----------------------------------
  // integration points & weights
  virtual const Mesh::vals GaussLegendre(const INT_ORDER )=0;
  const Mesh::vals GaussLegendre( ){
    return GaussLegendre( gaus_p ); };
#if 0
  int   SavePartFMR( const char* bname, bool is_bin );//FIXME ASCII/Binary file format
  Phys* ReadPartFMR( const char* bname, bool is_bin );
#endif
#ifdef HAS_PATCH
  virtual Mesh* Mesh1Natu( )=0;//FIXME These are defined in patch.cc
  virtual Mesh* MeshPatch( )=0;//FIXME Only first order
#endif
protected:
  Elem( INT_ORDER d, INT_MESH vn, INT_MESH en, INT_ORDER p, INT_MESH e ) :
    elem_d(d), elem_p(p), elem_vert_n(vn), elem_edge_n(en),
    elem_conn_n(vn+(p-1)*en), elem_n(e){// Elem Constructor
      gaus_p=p;
      elem_conn.resize( elem_n* elem_conn_n );
      elem_glid.resize( elem_n );
    };//FIXME Remove this one?
  Elem( INT_ORDER d, INT_MESH vn, INT_MESH en, INT_MESH ef,
        INT_ORDER p, INT_ORDER gp, INT_MESH e ) :// Elem Constructor
    elem_d(d), elem_p(p), gaus_p(gp), elem_vert_n(vn),
    elem_edge_n(en), elem_face_n(ef), elem_conn_n(vn+(p-1)*en+ef), elem_n(e){
      elem_conn.resize( elem_n* elem_conn_n );
      elem_glid.resize( elem_n );
    };
private:
};
class Bar final: public Elem{
public: Bar(INT_ORDER p) : Elem(1,2,1,p,1){}// Set properties in base Elem class
  const Mesh::vals ShapeFunction(const INT_ORDER, const FLOAT_MESH p[1])final;
  const Mesh::vals ShapeGradient(const INT_ORDER, const FLOAT_MESH  [1])final;
  const Mesh::vals GaussLegendre(const INT_ORDER p )final;
  int JacsDets()final;
#ifdef HAS_PATCH
  Mesh* Mesh1Natu()final;
  Mesh* MeshPatch()final;
#endif
protected:
  const Mesh::ints vert_conn={ 0,1 };
  const Mesh::ints vert_edge={ 0,1 };// Bar2 connectivity of 3 sides
  const Mesh::vals node_coor={-1.0,1.0};
private: 
};
class Tri final: public Elem{
public: Tri(INT_ORDER p) : Elem(2,3,3,p,1){}// Set properties in base Elem class
  const Mesh::vals ShapeFunction(const INT_ORDER, const FLOAT_MESH p[2])final;
  const Mesh::vals ShapeGradient(const INT_ORDER, const FLOAT_MESH  [2])final;
  const Mesh::vals GaussLegendre(const INT_ORDER p )final;
  int JacsDets()final;
#ifdef HAS_PATCH
  Mesh* Mesh1Natu()final;
  Mesh* MeshPatch()final;
#endif
protected:
  const Mesh::ints vert_conn={ 0,1,2 };
  const Mesh::ints vert_edge={ 0,1, 1,2, 2,0 };// Bar2 connectivity of 3 sides
  const Mesh::vals node_coor={
    0.0, 0.0,
    1.0, 0.0,
    0.0, 1.0};
private: 
};
class Qua final: public Elem{
public: Qua(INT_ORDER p) : Elem(2,4,4,p,1){}
  const Mesh::vals ShapeFunction(const INT_ORDER, const FLOAT_MESH p[2])final;
  const Mesh::vals ShapeGradient(const INT_ORDER, const FLOAT_MESH p[2])final;
  const Mesh::vals GaussLegendre(const INT_ORDER p )final;
  int JacsDets()final;
#ifdef HAS_PATCH
  Mesh* Mesh1Natu()final;
  Mesh* MeshPatch()final;
#endif
protected:
  const Mesh::ints vert_conn={ 0,1,2,3 };
  const Mesh::ints vert_edge={ 0,1, 1,2, 2,3, 3,0 };// Bar2 connectivity of 3 sides
  const Mesh::vals node_coor={
    -1.0,-1.0,
     1.0,-1.0,
     1.0, 1.0,
    -1.0, 1.0};
   //-1.0, 1.0, 1.0,-1.0,
   //-1.0,-1.0, 1.0, 1.0};
private:
};
class Tet final: public Elem{
public:
  Tet(INT_ORDER p) : Elem(3,4,6,((p<3)?0:4),p,p,1){  };//FIXME Remove?
  Tet(INT_ORDER p, INT_MESH e) : Elem(3,4,6,((p<3)?0:4),p,p,e){};
  const Mesh::vals ShapeFunction(const INT_ORDER, const FLOAT_MESH p[3])final;
  const Mesh::vals ShapeGradient(const INT_ORDER, const FLOAT_MESH  [3])final;
  const Mesh::vals GaussLegendre(const INT_ORDER p )final;
  int JacsDets()final;
#ifdef HAS_PATCH
  Mesh* Mesh1Natu()final;
  Mesh* MeshPatch()final;
#endif
protected:
  const Mesh::ints vert_conn={ 0,1,2,3 };
  //const RESTRICT Mesh::ints vert_edge={ 0,1, 1,2, 2,0, 0,3, 1,3, 2,3 };
  //const RESTRICT Mesh::ints vert_face={ 0,1,2, 0,1,3, 1,2,3, 2,0,3 }
  //NOTE The following match gmsh convention for quadratic tets
  const Mesh::ints vert_edge={ 0,1, 1,2, 2,0, 0,3, 2,3, 1,3 };
  const Mesh::ints vert_face={ 0,1,2, 0,1,3, 0,3,2, 1,2,3 };
  const Mesh::vals node_coor={
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0};
    //0.0, 1.0, 0.0, 0.0,
    //0.0, 0.0, 1.0, 0.0,
    //0.0, 0.0, 0.0, 1.0};
private:
};
class Bri final: public Elem{
public: Bri(INT_ORDER p) : Elem(3,8,12,p,1){}
  //Elem(elem_d, elem_n, elem_vert_n, elem_edge_n , elem_p)
  const Mesh::vals ShapeFunction(const INT_ORDER, const FLOAT_MESH p[3])final;
  const Mesh::vals ShapeGradient(const INT_ORDER, const FLOAT_MESH  [3])final;
  const Mesh::vals GaussLegendre(const INT_ORDER p )final;
  int JacsDets()final;
#ifdef HAS_PATCH
  Mesh* Mesh1Natu()final;
  Mesh* MeshPatch()final;
#endif
protected:
  const Mesh::ints vert_conn={ 0,1,2,3, 4,5,6,7 };
  const Mesh::ints vert_edge// Bar2 connectivity
    ={ 0,1, 1,2, 2,3,3,0, 4,5,5,6,6,7,7,4, 0,4,1,5,2,6,3,7 };
  const Mesh::ints vert_face
    ={ 0,1,2,3, 7,6,5,4, 0,4,5,1, 2,3,7,6, 0,3,7,4, 1,5,6,2 };
  const Mesh::vals node_coor={
                   //       7---------6
     0.0, 0.0, 0.0,//      /|        /|
     1.0, 0.0, 0.0,//     / |       / |
     1.0, 1.0, 0.0,//    /  |      /  |
     0.0, 1.0, 0.0,//   4---------5   |
     0.0, 0.0, 1.0,//   |   |     |   |
     1.0, 0.0, 1.0,//   |   3-----|---2
     1.0, 1.0, 1.0,//   |  /      |  /
     0.0, 1.0, 1.0 //   | /       | /
                   //   |/        |/
  };               //   0---------1
private:
};
//============= Inline Function Definitions ===============
// JD is jacobian dimension (1-3)
#define JD 1
inline FLOAT_MESH Elem::Jac1Det(const FLOAT_MESH m){
  return( m );
}
inline FLOAT_MESH Elem::Jac1Det(RESTRICT const Mesh::vals& m){
  return( m[0] );
}
inline int Elem::Jac1Inv(RESTRICT Mesh::vals& m, const FLOAT_MESH jacdet){
  if(jacdet==0){return 1;}
  m= static_cast<FLOAT_MESH>(1.0)/jacdet;
  if(jacdet < 0){return -1;} else{return 0;}
}
#undef JD
#define JD 2
inline FLOAT_MESH Elem::Jac2Det(const FLOAT_MESH* m){
  return( m[JD*0+ 0]*m[JD*1+ 1] - m[JD*0+ 1]*m[JD*1+ 0]);
}
inline FLOAT_MESH Elem::Jac2Det(const RESTRICT Mesh::vals& m){
  return( m[JD*0+ 0]*m[JD*1+ 1] - m[JD*0+ 1]*m[JD*1+ 0]);
}
inline int Elem::Jac2Inv( FLOAT_MESH* m, const FLOAT_MESH jacdet){
  if(jacdet==0){return 1;}
  FLOAT_MESH minv[4]; FLOAT_MESH dinv=static_cast<FLOAT_MESH>(1.0)/jacdet;
  for(int i=0;i<4;i++){ minv[i]=dinv; };
  minv[JD*0+ 0]*= m[JD*1+ 1] ;
  minv[JD*0+ 1]*=-m[JD*0+ 1] ;
  minv[JD*1+ 0]*=-m[JD*1+ 0] ;
  minv[JD*1+ 1]*= m[JD*0+ 0] ;
  //m=minv*(static_cast<FLOAT_MESH>(1.0)/jacdet);
  for(int i=0;i<4;i++){ m[i]=minv[i]; };
  if(jacdet < 0){return -1;} else{return 0;}
}
inline int Elem::Jac2Inv(RESTRICT Mesh::vals& m, const FLOAT_MESH jacdet){
  if(jacdet==0){return 1;}
  RESTRICT Mesh::vals minv(4);
  minv[JD*0+ 0] = m[JD*1+ 1] ;
  minv[JD*0+ 1] =-m[JD*0+ 1] ;
  minv[JD*1+ 0] =-m[JD*1+ 0] ;
  minv[JD*1+ 1] = m[JD*0+ 0] ;
  m=minv*(static_cast<FLOAT_MESH>(1.0)/jacdet);
  if(jacdet < 0){return -1;} else{return 0;}
}
inline int Elem::Jac2Tnv(RESTRICT Mesh::vals& m, const FLOAT_MESH jacdet){
  if(jacdet==0){return 1;}
  RESTRICT Mesh::vals minv(4);
  minv[JD*0+ 0] = m[JD*1+ 1] ;
  minv[JD*0+ 1] =-m[JD*1+ 0] ;
  minv[JD*1+ 0] =-m[JD*0+ 1] ;
  minv[JD*1+ 1] = m[JD*0+ 0] ;
  m=minv*(static_cast<FLOAT_MESH>(1.0)/jacdet);
  if(jacdet < 0){return -1;} else{return 0;}
}
#undef JD
#define JD 3
inline FLOAT_MESH Elem::Jac3Det(const FLOAT_MESH* m ){
  return(
      m[JD*0+ 0]*(m[JD*1+ 1] * m[JD*2+ 2] - m[JD*2+ 1] * m[JD*1+ 2])
    + m[JD*0+ 1]*(m[JD*1+ 2] * m[JD*2+ 0] - m[JD*2+ 2] * m[JD*1+ 0])
    + m[JD*0+ 2]*(m[JD*1+ 0] * m[JD*2+ 1] - m[JD*2+ 0] * m[JD*1+ 1]) );
}
inline FLOAT_MESH Elem::Jac3Det(RESTRICT const Mesh::vals& m){
  return(
      m[JD*0+ 0]*(m[JD*1+ 1] * m[JD*2+ 2] - m[JD*2+ 1] * m[JD*1+ 2])
    + m[JD*0+ 1]*(m[JD*1+ 2] * m[JD*2+ 0] - m[JD*2+ 2] * m[JD*1+ 0])
    + m[JD*0+ 2]*(m[JD*1+ 0] * m[JD*2+ 1] - m[JD*2+ 0] * m[JD*1+ 1]) );
}
inline int Elem::Jac3Inv( FLOAT_MESH* m, const FLOAT_MESH jacdet){
  if(jacdet==0){return 1;}
  //returns inverse in m
  FLOAT_MESH minv[9]; FLOAT_MESH dinv=static_cast<FLOAT_MESH>(1.0)/jacdet;
  for(int i=0;i<9;i++){ minv[i]=dinv; };
  //
  minv[JD*0+ 0]*= (m[JD*1+ 1] * m[JD*2+ 2] - m[JD*2+ 1] * m[JD*1+ 2]) ;
  minv[JD*0+ 1]*= (m[JD*0+ 2] * m[JD*2+ 1] - m[JD*0+ 1] * m[JD*2+ 2]) ;
  minv[JD*0+ 2]*= (m[JD*0+ 1] * m[JD*1+ 2] - m[JD*0+ 2] * m[JD*1+ 1]) ;
  //
  minv[JD*1+ 0]*= (m[JD*1+ 2] * m[JD*2+ 0] - m[JD*1+ 0] * m[JD*2+ 2]) ;
  minv[JD*1+ 1]*= (m[JD*0+ 0] * m[JD*2+ 2] - m[JD*0+ 2] * m[JD*2+ 0]) ;
  minv[JD*1+ 2]*= (m[JD*1+ 0] * m[JD*0+ 2] - m[JD*0+ 0] * m[JD*1+ 2]) ;
  //
  minv[JD*2+ 0]*= (m[JD*1+ 0] * m[JD*2+ 1] - m[JD*2+ 0] * m[JD*1+ 1]) ;
  minv[JD*2+ 1]*= (m[JD*2+ 0] * m[JD*0+ 1] - m[JD*0+ 0] * m[JD*2+ 1]) ;
  minv[JD*2+ 2]*= (m[JD*0+ 0] * m[JD*1+ 1] - m[JD*1+ 0] * m[JD*0+ 1]) ;
  //m=minv;//*(static_cast<FLOAT_MESH>(1.0)/jacdet);
  for(int i=0;i<9;i++){ m[i]=minv[i]; };
  if(jacdet < 0){return -1;} else{return 0;}
}
inline int Elem::Jac3Inv(RESTRICT Mesh::vals& m, const FLOAT_MESH jacdet){
  if(jacdet==0){return 1;}
  //returns inverse in m
  RESTRICT Mesh::vals minv(static_cast<FLOAT_MESH>(1.0)/jacdet,9);
  //
  minv[JD*0+ 0]*= (m[JD*1+ 1] * m[JD*2+ 2] - m[JD*2+ 1] * m[JD*1+ 2]) ;
  minv[JD*0+ 1]*= (m[JD*0+ 2] * m[JD*2+ 1] - m[JD*0+ 1] * m[JD*2+ 2]) ;
  minv[JD*0+ 2]*= (m[JD*0+ 1] * m[JD*1+ 2] - m[JD*0+ 2] * m[JD*1+ 1]) ;
  //
  minv[JD*1+ 0]*= (m[JD*1+ 2] * m[JD*2+ 0] - m[JD*1+ 0] * m[JD*2+ 2]) ;
  minv[JD*1+ 1]*= (m[JD*0+ 0] * m[JD*2+ 2] - m[JD*0+ 2] * m[JD*2+ 0]) ;
  minv[JD*1+ 2]*= (m[JD*1+ 0] * m[JD*0+ 2] - m[JD*0+ 0] * m[JD*1+ 2]) ;
  //
  minv[JD*2+ 0]*= (m[JD*1+ 0] * m[JD*2+ 1] - m[JD*2+ 0] * m[JD*1+ 1]) ;
  minv[JD*2+ 1]*= (m[JD*2+ 0] * m[JD*0+ 1] - m[JD*0+ 0] * m[JD*2+ 1]) ;
  minv[JD*2+ 2]*= (m[JD*0+ 0] * m[JD*1+ 1] - m[JD*1+ 0] * m[JD*0+ 1]) ;
  m=minv;//*(static_cast<FLOAT_MESH>(1.0)/jacdet);
  if(jacdet < 0){return -1;} else{return 0;}
}
inline int Elem::Jac3Tnv(RESTRICT Mesh::vals& m, const FLOAT_MESH jacdet){
  if(jacdet==0){return 1;}
  //returns transposed inverse in m
  RESTRICT Mesh::vals minv(static_cast<FLOAT_MESH>(1.0)/jacdet,9);
  // minv= 1 / jacdet; // inverse of matrix m
  //
  minv[JD*0+ 0]*= (m[JD*1+ 1] * m[JD*2+ 2] - m[JD*2+ 1] * m[JD*1+ 2]) ;
  minv[JD*0+ 1]*= (m[JD*1+ 2] * m[JD*2+ 0] - m[JD*1+ 0] * m[JD*2+ 2]) ;
  minv[JD*0+ 2]*= (m[JD*1+ 0] * m[JD*2+ 1] - m[JD*2+ 0] * m[JD*1+ 1]) ;
  //
  minv[JD*1+ 0]*= (m[JD*0+ 2] * m[JD*2+ 1] - m[JD*0+ 1] * m[JD*2+ 2]) ;
  minv[JD*1+ 1]*= (m[JD*0+ 0] * m[JD*2+ 2] - m[JD*0+ 2] * m[JD*2+ 0]) ;
  minv[JD*1+ 2]*= (m[JD*2+ 0] * m[JD*0+ 1] - m[JD*0+ 0] * m[JD*2+ 1]) ;
  //
  minv[JD*2+ 0]*= (m[JD*0+ 1] * m[JD*1+ 2] - m[JD*0+ 2] * m[JD*1+ 1]) ;
  minv[JD*2+ 1]*= (m[JD*1+ 0] * m[JD*0+ 2] - m[JD*0+ 0] * m[JD*1+ 2]) ;
  minv[JD*2+ 2]*= (m[JD*0+ 0] * m[JD*1+ 1] - m[JD*1+ 0] * m[JD*0+ 1]) ;
  m=minv;//*(static_cast<FLOAT_MESH>(1.0)/jacdet);
  if(jacdet < 0){return -1;} else{return 0;}
}
#undef JD
//============= Inline Template Definitions ===============
template <typename F> static inline
  int part_resp_glob( Elem* E, Phys* Y, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV* RESTRICT part_f, const FLOAT_SOLV* RESTRICT part_u, F mtrl_resp ){
  //FIXME Clean up local variables.
  //const int De = 3;// Element Dimension
  const int Nd = 3;// Node (mesh) Dimension
  const int Nf = 3;// Y->node_d DOF/node
  const int Nj = Nd*Nd+1;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nf*Nc;
  const int Nt = 4*Nc;
  const int intp_n = int(E->gaus_n);
  //const INT_ORDER elem_p =E->elem_p;
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
#ifdef THIS_FETCH_JAC
  FLOAT_MESH VECALIGNED jac[Nj];
#endif
  FLOAT_PHYS VECALIGNED G[Nt], u[Ne];
  //
  FLOAT_MESH VECALIGNED data_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED data_weig[intp_n];
  uint s=( Y->mtrl_dmat.size() > Y->mtrl_matc.size() )
    ? Y->mtrl_dmat.size() : Y->mtrl_matc.size() ;
  FLOAT_PHYS VECALIGNED data_matc[s];
  //FLOAT_PHYS VECALIGNED data_dmat[Y->mtrl_dmat.size()];
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], data_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], data_weig );
  if(s==48){
    std::copy( &Y->mtrl_dmat[0], &Y->mtrl_dmat[Y->mtrl_dmat.size()], data_matc );
  }else{
    std::copy( &Y->mtrl_matc[0], &Y->mtrl_matc[Y->mtrl_matc.size()], data_matc );
  }
  //
  const VECALIGNED FLOAT_MESH* RESTRICT shpg = &data_shpg[0];
  const VECALIGNED FLOAT_SOLV* RESTRICT wgt  = &data_weig[0];
  const VECALIGNED FLOAT_SOLV* RESTRICT C    = &data_matc[0];
  //
#if VERB_MAX>11
  printf( "Material [%u]:", (uint)Y->mtrl_matc.size() );
  for(uint j=0;j<Y->mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  } printf("\n");
#endif
  if(e0<ee){
#ifdef THIS_FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT c = &Econn[Nc*e0];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i],&part_u[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf ); }
  }
  for(INT_MESH ie=e0;ie<ee;ie++){//================================== Elem loop
#ifdef THIS_FETCH_JAC
    const __m256d vJ[3]={
      _mm256_load_pd(&jac[0]),   // j0 = [j3 j2 j1 j0]
      _mm256_loadu_pd(&jac[3]),  // j1 = [j6 j5 j4 j3]
      _mm256_loadu_pd(&jac[6]) };// j2 = [j9 j8 j7 j6]
#else
    const __m256d vJ[3]={
      _mm256_loadu_pd(&Ejacs[Nj*ie  ]),  // j0 = [j3 j2 j1 j0]
      _mm256_loadu_pd(&Ejacs[Nj*ie+3]),  // j1 = [j6 j5 j4 j3]
      _mm256_loadu_pd(&Ejacs[Nj*ie+6]) };// j2 = [j9 j8 j7 j6]
    const FLOAT_MESH det=Ejacs[Nj*ie+9];
#endif
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    {// Scope vf registers
    __m256d vf[Nc];
#ifdef FETCH_F_EARLY
    for(int i=0; i<Nc; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
#endif
    for(int ip=0; ip<intp_n; ip++){//============================== Int pt loop
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      __m256d vH[3];
      compute_g_h( &G[0],&vH[0], Nc, &vJ[0], &shpg[ip*Ne], &u[0] );
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<H.size();j++){
        if(j%Nd==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      } printf("\n");
#endif
#ifdef THIS_FETCH_JAC
      const FLOAT_PHYS dw = jac[9] * wgt[ip];
#else
      const FLOAT_PHYS dw = det * wgt[ip];
#endif
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
#ifndef FETCH_U_EARLY
        const INT_MESH* RESTRICT cnxt = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
        for (int i=0; i<Nc; i++){
          std::memcpy(& u[Nf*i],& part_u[cnxt[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); }
#endif
#ifdef THIS_FETCH_JAC
          std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj );
#endif
      } }
#if 0
      compute_iso_s( &vH[0], C[1]*dw,C[2]*dw );// Reuse vH instead of new vS
#else
      mtrl_resp( &vH[0], &C[0], dw );// Reuse vH instead of new vS
#endif
#ifndef FETCH_F_EARLY
      if(ip==0){
        for(int i=0; i<Nc; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
      }
#endif
      accumulate_f( &vf[0], &vH[0], &G[0], Nc );
    }//========================================================== end intp loop
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
    for (int i=0; i<Nc; i++){
      double VECALIGNED sf[4];
      _mm256_store_pd(&sf[0],vf[i]);
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for(int j=0; j<3; j++){
        part_f[3*conn[i]+j] = sf[j]; } }
    }// end vf register scope
  }//============================================================ end elem loop
  return 0;
}
#if 0
template <typename F> static inline
  int part_loop( std::vector<Mesh::part> P, Elem* E, Phys* Y, Solv* S,
    FLOAT_SOLV* halo_vals,
    int part_0, int part_o, F solv_krnl ){
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=P[part_i];
    const INT_MESH Dm=uint(E->mesh_d);
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hnn=E->halo_node_n,hl0=S->halo_loca_0,sysn=S->udof_n;
    solv_krnl( E,Y,S, halo_vals, Dm,Dn, hnn, hl0, sysn );
  }
  return 0;
}
template <typename F> static inline
  int part_sum1( std::vector<Mesh::part> P, Elem* E, Phys* Y, Solv* S,
    FLOAT_SOLV* halo_vals,
    int part_0, int part_o, FLOAT_SOLV redu_summ, F solv_krnl ){
#pragma omp for schedule(static) reduction(+:redu_summ)
  for(int part_i=part_0; part_i<part_o; part_i++){
    std::tie(E,Y,S)=P[part_i];
    const INT_MESH Dm=uint(E->mesh_d);
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hnn=E->halo_node_n,hl0=S->halo_loca_0,sysn=S->udof_n;
    solv_krnl( E,Y,S, halo_vals, Dm,Dn, hnn, hl0, sysn, redu_summ );
  }
  return 0;
}
#endif
//
#endif
