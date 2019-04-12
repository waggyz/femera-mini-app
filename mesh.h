#ifndef INCLUDED_MESH_H
#define INCLUDED_MESH_H
#include <tuple>
#include <vector>
#include <set>
#include <unordered_map>
#include <array>
//
class Mesh{
public:
  // valarray: variable-length numerical c++ type
  // Intel's compiler vectorizes pre-defined numerical operations
  typedef std::valarray<FLOAT_MESH> vals;
  typedef std::valarray<  INT_MESH> ints;//FIXME Best choice for this? Maybe vals is better?
  //
  //FIXME Move these into Elem? -------------------------------------
  //FIXME nodeID should match gmsh?
  // < node_id, dof_id, value >  and < node_id, dof_id >:
  typedef std::tuple<INT_MESH,INT_DOF,FLOAT_SOLV> nfval;
  typedef std::set  <nfval> nfvals;// for RHS [and fixed-nonzero BCs]
  typedef std::tuple<INT_MESH,INT_DOF> nfitem;
  typedef std::set  <nfitem> nflist;// for fixed-zero boundary conditions
  //-----------------------------------------------------------------
  typedef std::tuple<Elem*,Phys*,Solv*> part;
  //
  vals vert_coor={};//NOTE May not be populated (stored in elem->vert_coor)
  //vals side_coor={};//NOTE Normally not populated (calculated when needed)
  //
  //FIXME Not needed? Mesh partitioning local part & node number xref lookup tables.
  //ints node_part    ={};// xref of which partition a global node is in
  //ints node_part_id ={};// xref from global to local node number
  // local_id=node_part_id[global_id];
  //NOTE Global node numbers are 1-indexed.
  //NOTE Local (partition) node numbers are 0-indexed.
  //
  //
  INT_ORDER mesh_d=3;//FIXME mesh_d_n?
  INT_MESH  vert_n=0;// Total number of element vertex nodes
  INT_MESH  node_n=0;// Total number of nodes
  INT_MESH  elem_n=0;// Total number of nodes
  INT_MESH  udof_n=0;// Total number of solution values (size of system)
  uint      simd_n=1;// Initial vectorization width
  //
  int comp_n=1, numa_n=1;
  int solv_meth=Solv::SOLV_CG, solv_cond=Solv::COND_JACO, verbosity=1;
  int hown_meth=2;
  //
  //FIXME Consider moving these to Test*
  float phys_flop=0, phys_band=0;
  float solv_flop=0, solv_band=0;
  //
  int iter_max=0;
  FLOAT_SOLV glob_rtol=0.0, glob_rto2=0.0, glob_chk2=0.0, glob_res2=0.0;
  //FLOAT_SOLV solv_rtol=0.0, rtol_pow2=0.0, resi_chk2=0.0, resi_pow2=0.0;//, resb_pow2;
  //
  //FIXED Change these to < node_id, dof_id, value >  and < node_id, dof_id >:
  //FIXME Move these into Elem?
  //NOTE These are by global ID
  nfvals rhs_vals={};// Nodal forces applied (nonzeros)
  nfvals bcs_vals={};// Dirichlet boundary conditions (nonzeros)
  nflist bc0_nf  ={};// Essential (u=0) Dirichlet BCs (node_id,dof_id) tuples
  //*****************************************************************
  std::vector<Elem*> list_elem={};//FIXME Remove.
  std::vector<part>  mesh_part={};
  //std::unordered_map<int,std::array<FLOAT_MESH,3>> halo_coor;
  // Used by Sync/Gather/ScatterHaloCoor
  //
  std::unordered_map<INT_MESH,INT_MESH> halo_map;
  std::valarray<FLOAT_SOLV> halo_val={};//(ndof_n*halo_loca_tot);
  std::valarray<float> time_secs={};
  //
  std::string base_name = "";// base of partitioned mesh filename (without _i)
  std::string meth_name = "";// solution method name
  //
  //int InitPCG();
  //int IterPCG();
  //int InitPCR();
  //int IterPCR();
  //
  //int GatherHaloCoor();
  //int ScatterHaloCoor();
  //int SyncHaloCoor();
  //
  int Setup();//FIXME Not yet used?
  virtual int Init()=0;
  virtual int Iter()=0;
  int GatherGlobalIDs();
  int ScatterHaloIDs();
  int SyncIDs();
  //
  //virtual Elem* ReadFile( const char* fname, INT_ORDER pord )=0;
  //
protected:
  Mesh( Solv::Meth m ) : solv_meth(m){};
  Mesh( Solv::Meth m, INT_MESH i, FLOAT_PHYS r ) :
    solv_meth(m),iter_max(i),glob_rtol(r){};
  Mesh( int p, Solv::Meth m, INT_MESH i, FLOAT_PHYS r ) :
    solv_meth(m),iter_max(i),glob_rtol(r){
      mesh_part.resize(p);
    };
private:
};
class HaloPCG final: public Mesh{
// Preconditioned Conjugate Gradient Kernel ----------------------------
public:
  HaloPCG() : Mesh(Solv::SOLV_CG){
    meth_name="preconditioned cojugate gradient";
  };
  HaloPCG( INT_MESH i, FLOAT_PHYS r ) : Mesh(Solv::SOLV_CG,i,r){
    meth_name="preconditioned cojugate gradient";
  };
  HaloPCG( int p, INT_MESH i, FLOAT_PHYS r ) : Mesh(p,Solv::SOLV_CG,i,r){
    meth_name="preconditioned cojugate gradient";
  };
  int Init() final;
  int Iter() final;
protected:
private:
};
class HaloPCR final: public Mesh{
// Preconditioned Conjugate Residual Kernel ----------------------------
public:
  HaloPCR() : Mesh(Solv::SOLV_CR){
    meth_name="preconditioned cojugate residual";
  };
  HaloPCR( INT_MESH i, FLOAT_PHYS r ) : Mesh(Solv::SOLV_CR,i,r){
    meth_name="preconditioned cojugate residual";
  };
  HaloPCR( int p, INT_MESH i, FLOAT_PHYS r ) : Mesh(p,Solv::SOLV_CR,i,r){
    meth_name="preconditioned cojugate residual";
  };
  int Init() final;
  int Iter() final;
protected:
private:
};
//Elem* Mesh::ReadFile( const char* fname, INT_ORDER pord ){};
/*
class Msh final: public Mesh{
public:
  Elem* ReadFile( const char* fname, INT_ORDER pord )final{return 0;};

protected:
private:
};
*/
/*
class Gmsh final: public Mesh{
public:
  Elem* ReadFile( const char* fname, INT_ORDER pord );

protected:
private:
};
*/




#endif
