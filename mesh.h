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
  vals node_coor={};//NOTE May not be populated (stored in elem->node_coor)
  //NOTE Global node numbers are 1-indexed.
  //NOTE Local (partition) node numbers are 0-indexed.
  //
  INT_DIM  mesh_d=3;// Mesh Dimension (usually max of elem_d)
  INT_MESH vert_n=0;// Total number of element vertex nodes
  INT_MESH node_n=0;// Total number of nodes
  INT_MESH elem_n=0;// Total number of nodes
  INT_MESH udof_n=0;// Total number of solution values (size of system)
  //
  uint     simd_n=1;//FIXME Remove? Initial vectorization width
  //
  int comp_n=1;
  int solv_meth=Solv::SOLV_CG, solv_cond=Solv::COND_JACO, verbosity=1;
  int hown_meth=2;// Halo node ownership method
  //
  INT_MESH iso3_part_n=0;
  INT_MESH ort3_part_n=0;
  INT_MESH ther_part_n=0;
  INT_MESH plas_part_n=0;
  //
  FLOAT_SOLV cube_init=0.0;// Start u0 at scaled iso cube exact solution
  //FIXME Should only be in Solv* objects?
  //
  int iter_max=0;
  int load_step=1, load_step_n=1;
  FLOAT_SOLV step_scal = 1.0, next_scal=1.0;
  //
  FLOAT_SOLV glob_rtol= 0.0, glob_rto2=0.0, glob_chk2=0.0, glob_res2=0.0;
  FLOAT_SOLV glob_atol=-1.0, glob_ato2=0.0;
  FLOAT_SOLV glob_otol= 0.1;// Sierra Mechanics (non)orthogonality tolerance
  //
  //FIXME Consider moving these to Test*
  float phys_flop=0, phys_band=0;
  float solv_flop=0, solv_band=0;
  //
  FLOAT_MESH glob_bbox[6]={9e9,9e9,9e9 , -9e9,-9e9,-9e9};// Bounding box
  FLOAT_SOLV glob_bmax[4]={0.0,0.0,0.0,0.0};// Boundary conditions max magnitude
  //
  //NOTE These are by global ID
  nfvals rhs_vals={};// Nodal forces applied (nonzeros)
  nfvals bcs_vals={};// Dirichlet boundary conditions (nonzeros)
  nflist bc0_nf  ={};// Essential (u=0) Dirichlet BCs (node_id,dof_id) tuples
  //*****************************************************************
  std::vector<Elem*> list_elem={};//FIXME Remove.
  std::vector<part>  mesh_part={};
  //
  std::unordered_map<INT_MESH,INT_MESH> halo_map;
  std::valarray<FLOAT_SOLV> halo_val={};
  std::valarray<float> time_secs={};
  //
  std::string base_name = "";// base of partitioned mesh filename (without _i)
  std::string meth_name = "";// solution method name
  //FIXME ASCII/Binary file format
  int SavePartFMR( const part&, const char* bname, bool is_bin );
  int ReadPartFMR( part&, const char* bname, bool is_bin );
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
#if OMP_NESTED!=true
  static std::vector<part> priv_part;
#pragma omp threadprivate(priv_part)
#endif
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
class HaloNCG final: public Mesh{
// Preconditioned Conjugate Gradient Kernel ----------------------------
public:
  HaloNCG() : Mesh(Solv::SOLV_NG){
    meth_name="nonlinear cojugate gradient";
  };
  HaloNCG( INT_MESH i, FLOAT_PHYS r ) : Mesh(Solv::SOLV_NG,i,r){
    meth_name="nonlinear cojugate gradient";
  };
  HaloNCG( int p, INT_MESH i, FLOAT_PHYS r ) : Mesh(p,Solv::SOLV_NG,i,r){
    meth_name="nonlinear cojugate gradient";
  };
  int Init() final;
  int Iter() final;
protected:
private:
};
#if 0
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

#endif
