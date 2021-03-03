#ifndef FMR_HAS_DATATYPE_HPP
#define FMR_HAS_DATATYPE_HPP

#include "math.hpp"

#include <valarray>
#include <map>
#include <unordered_map>
#include <functional>        // std::hash

namespace fmr {//TODO? namespace data {
  /* These typedefs, structs, and static stuff are used throughout
   * fmr:: and Femera:: namespaces to manage data in memory and stored in files.
   * */
  typedef std::string Data_id;
  typedef std::vector<fmr::Local_int> Tree_path;
  enum class Tree_type : Enum_int {None=0, Error, Unknown,
    Mtrl, Sims, Gset, Part, Join_part, Mesh, Grid,
  end};
  typedef std::unordered_map<Global_int,Local_int> Node_local_map;
  typedef std::unordered_map<Global_int,Local_int> Elem_local_map;
  enum class Data : Enum_int {None=0, Error, Unknown,// Homogeneous data types
    Sims_n,                // Local_int
    // Geometry datatypes ----------------------------------------------------
    Gset_n,                // Local_int  [Sims_n]
    Geom_d,                // Dim_int    [Sims/Part/Mesh_n] enclosing space dim
    Part_n,                // Local_int  [Sims/Part_n]
    Part_halo_n,           // Local_int  [Sims/Part_n] shared surfaces
    Mesh_halo_n,           // Local_int  [Part_n] shared surfaces
    Grid_halo_n,           // Local_int  [Part_n] shared surfaces
    //
    // Data for both unstructured meshes and structured grids
    Geom_n,                // Local_int  [Part_n] Geom_n=Mesh_n+Grid_n
    Axis_rots_type,        // Enum_int   [Geom_n]
    Axis_rots,             // Geom_float [Geom_n, 1/3] Euler angle(s)
    Axis_quat,             // Geom_float [Geom_n, 4] fmr::math::Unit_quaternion
    Node_n,                // Local_int  [Geom_n]
    Node_x, Node_y, Node_z,// Geom_float [Node_n]
    Node_coor,             // Geom_float [Node_n, Elem/Mesh_d]
    //
    // Unstructured (finite element) mesh data //TODO boundary element method
    Mesh_n,                // Local_int  [Part_n]
    Elem_n,                // Local_int  [Mesh_n]
    Elem_d, Elem_p,        // Dim_int    [Mesh_n]
    Elem_type, Elem_poly,  // Enum_int   [Mesh_n]
    // Elemid_int elem_id = make_elem_id (elem_type, elem_poly, elem_p);
    Elem_id,               // Elemid_int [Mesh_n]
    Elem_vrtx_n,           // Dim_int    [Mesh_n]// only vertex (corner) nodes
    Elem_node_n,           // Local_int  [Mesh_n]// vrtx+edge+face+body nodes
    Elem_jacs_n,           // Local_int  [Mesh_n]
    Jacs_type,             // Enum_int   [Mesh_n]
    Jacs_size_n,           // Dim_int    [Mesh_n] typically 1D:1, 2D:4, 3D:10
    Jacs_dets,             // Geom_float [Elem_n, Elem_jacs_n, Jacs_size_n]
    Node_global_id,        // Global_int [Node_n]
    Elem_global_id,        // Global_int [Elem_n]
    Elem_conn,             // Global/Local_int [Elem_n, Elem_node_n]
    // Build Global_id <=> Local_id maps while reading Elem_conn<Global_int>.
    //
    // Structured grid data
    Grid_n,                // Local_int  [Part_n]
    Cell_d,                // Dim_int    [Grid_n]
    Cell_type,             // Enum_int   [Grid_n]
    Grid_cell_n,           // Local_int  [Grid_n, Cell_d]
    Cell_size,             // Geom_float [Grid_n, Cell_d]
    // Physics datatypes -----------------------------------------------------
    Sims_type, Time_type,  // Enum_int   [Sims_n]
    Phys_d,                // Dim_int    [Sims/Part/Mesh_n]
    Mtrl_n,                // Local_int  [Sims/Part/Mesh_n]
    Phys_type,             // Enum_int   [Mtrl_n]
    // Load and boundary condition datatypes ---------------------------------
    // Solver datatypes ------------------------------------------------------
    // Post-processing datatypes ---------------------------------------------
#if 1
    // Heterogeneous datatypes -----------------------------------------------
    //TODO Remove heterogeneous data types. Add back if sync or distribution
    //     is needed.
    // Heterogenous data types: packed as Enum_int, Local_int, or Global_int.
    // The enum items have a corresponding fmr:: class below.
    //
    Elem_ig, Node_ig,
    Geom_info,// Dim_int, Enum_int, Local_int vals
    Phys_info,// Combine with Geom_info as Part_info?
#endif
  end};// The last item is "end" to indicate size
  typedef std::pair<std::string,std::string> Name_pair;
  static const std::map<Data,Name_pair> Data_name {
    {Data:: None     , Name_pair{"data_none","no data"}},
    {Data:: Error    , Name_pair{"data_err" ,"data error"}},
    {Data:: Unknown  , Name_pair{"data_unk" ,"unknown data"}},
    {Data:: Node_x   , Name_pair{"node_x"   ,"node x-ordinates"}},// Geom_float
    {Data:: Node_y   , Name_pair{"node_y"   ,"node y-ordinates"}},// Geom_float
    {Data:: Node_z   , Name_pair{"node_z"   ,"node z-ordinates"}},// Geom_float
    {Data:: Node_coor, Name_pair{"node_coor","node coordinates"}},// Geom_float
    {Data:: Jacs_dets, Name_pair{
      "jacs_dets","element jacobians and determinants"}},// Geom_float
  };
#if 1
  //TODO Remove these heterogenous data types. Add back if sync is needed.
  /* Human-readable (enum) index names for heterogenous data types above
   * isok: for packing a bit array indicating which items have valid data
   * The last item is always "end" to indicate size in the Data_size map below.
  */
  enum class Geom_info : Enum_int {isok=0,// Use for Sims and Part data.
    Gset_n, Part_n, Mesh_n, Grid_n,       // Local_int
    Part_halo_n, Mesh_halo_n, Grid_halo_n,// Local_int
    Geom_d,                               // Dim_int
  end};
  enum class Phys_info : Enum_int {isok=0,
    Mtrl_n,  // Local_int //TODO here?
    Sim_time,// Enum_int
    Phys_d,  // Dim_int
  end};
  struct Math_size {// Size and algebra type of data.
    size_t           size = 0;
    math::Zomplex algebra = math::Real;
    // constructor
    Math_size (const math::Zomplex z, const size_t sz) : size(sz), algebra(z){}
  };
  //NOTE For compile-time lookup use Data_size.at(type),
  //     NOT Data_size[type] nor Data_size.find(type).
  static const std::map<Data, Math_size> Data_size {//TODO Remove.
    // for heterogeneous data
    {Data::Geom_info, Math_size(math::Natural, enum2val (Geom_info::end))},
    {Data::Phys_info, Math_size(math::Natural, enum2val (Phys_info::end))}
  };
#endif
  enum class Elem : Enum_int {None=0, Error, Unknown,
    Bars,    // Beam,
    Tris,    // Quad,
    Tets,    // Pymd, Prsm, Cube,
    // Mbrn, Shll_xxxx
    // Cell_FD, Cell_FV,
    // FD1D, FD2D, FD3D, FV1D, FV2D, FV3D, SG1D, SG2D, SG3D, // grid cell types
  end};
  inline Elemid_int make_elem_id (Elem e, fmr::math::Poly y, fmr::Dim_int p){
    constexpr auto ysz = 8*sizeof(y), psz = 8*sizeof(p);
    return
      ( (Elemid_int(fmr::enum2val(e)) << (ysz+psz))
      + (Elemid_int(fmr::enum2val(y)) << (psz)) + p );
  }
}//end fmr:: namespace -------------------------------------------------------
namespace fmr {namespace data {//TODO Move to vals.hpp?
  struct State {
    bool is_default  = false;//  do_use_default
    bool can_write   = false;// !is_read_only, can_modify
    bool can_read    = false;// !is_write_only
    bool was_read    = false;// !is_new, is_stored, was_found, is_in_file
    bool was_checked = false;// !is_unknown, has_been_checked
    bool has_changed = false;//  has_changed_on_disk, has_changed_in_memory
    bool do_save     = false;// Save data before closing/freeing.
    bool has_error   = false;// !is_ok
#if 0
    bool do_free     = false;// Data handler or user frees memory.//TODO needed?
#endif
  };
  inline bool is_loaded_ok (const State S){
    return S.was_read && !S.has_error;
  }
} }//end fmr::data:: namespace -----------------------------------------------
//end FMR_HAS_DATATYPE_HPP
#endif
