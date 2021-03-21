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
  enum class Data : Enum_int {
    None=0, Error, Unknown,
    // included content from data/vals-type.csv using tools/vals-type.sh
#include "data-enum.inc"
#if 0
    Sims_n,                // Local_int
    // Geometry datatypes ----------------------------------------------------
    Gset_n,                // Local_int  [Sims_n]
    Geom_d,                // Dim_int    [Sims/Part/Mesh_n] enclosing space dim
    Sims_size,             // Enum_int   [Sims_n]//TODO Size_type ?
    Part_type,             // Enum_int   [Part_n]
    Part_n,                // Local_int  [Sims/Part_n]
    //
    // Data for both unstructured meshes and structured grids
    Geom_n,                // Local_int  [Part_n] Geom_n=Mesh_n+Grid_n
    Node_n,                // Local_int  [Geom_n]
    Node_x, Node_y, Node_z,// Geom_float [Node_n]
    Node_coor,             // Geom_float [Node_n, Elem/Mesh_d]
    Rots_type,             // Enum_int   [Geom_n]
    Axis_rots,             // Geom_float [Geom_n, 1/3] Euler angle(s)
    Axis_quat,             // Geom_float [Geom_n, 4] fmr::math::Unit_quaternion
    //
    // Unstructured (finite element) mesh data //TODO boundary element method
    Mesh_n,                // Local_int  [Part_n]
    Elem_n,                // Local_int  [Mesh_n]
    Elem_type,             // Enum_int   [Mesh_n]
#if 0
    Elem_conn,             // Global/Local_int [Elem_n, Elem_node_n]
    Poly_type,             // Enum_int   [Mesh_n]
    Elem_d, Elem_p,        // Dim_int    [Mesh_n]
    // Elid_int elid = make_elid (elem_form, elem_poly, elem_p);//form is shape
    Elem_elid,             // Elid_int   [Mesh_n]
    Elem_vert_n,           // Dim_int    [Mesh_n]// only vertex (corner) nodes
#endif
    Elem_node_n,           // Local_int  [Mesh_n]// vrtx+edge+face+body nodes
    Elem_jacs_n,           // Local_int  [Mesh_n]
    Jacs_type,             // Enum_int   [Mesh_n]
    Jacs_size,             // Dim_int    [Mesh_n] typically 1D:1, 2D:4, 3D:10
    Jacs_dets,             // Geom_float [Elem_n, Elem_jacs_n, Jacs_size]
    Node_id,               // Global_int [Node_n]
    Elem_id,               // Global_int [Elem_n]
    // Build Global_id <=> Local_ix maps while reading Elem_conn<Global_int>.
    //
    // Structured grid data
    Grid_n,                // Local_int  [Part_n]
    Cell_d,                // Dim_int    [Grid_n]
    Cell_type,             // Enum_int   [Grid_n]
    Grid_dims,             // Local_int  [Grid_n, Cell_d]
    Cell_size,             // Geom_float [Grid_n, Cell_d]
    // Sync data
    Part_halo_n,           // Local_int  [Sims/Part_n] shared surfaces
    Mesh_halo_n,           // Local_int  [Part_n] shared surfaces
    Grid_halo_n,           // Local_int  [Part_n] shared surfaces
    Node_halo_n,           // Local_int  [Geom_n]
    Elem_halo_n,           // Local_int  [Mesh_n]
    // Physics datatypes -----------------------------------------------------
    Sims_type, Time_type,  // Enum_int   [Sims_n]
    Phys_d,                // Dim_int    [Sims/Part/Mesh_n]
    Mtrl_n,                // Local_int  [Sims/Part/Mesh_n]
    Phys_type,             // Enum_int   [Mtrl_n]
    // Load and boundary condition datatypes ---------------------------------
    // Solver datatypes ------------------------------------------------------
    // Post-processing datatypes ---------------------------------------------
#endif
#if 1
    // Heterogeneous datatypes -----------------------------------------------
    //TODO Remove heterogeneous data types. Add back if sync or distribution
    //     is needed.
    // Heterogenous data types: packed as Enum_int, Local_int, or Global_int.
    // The enum items have a corresponding fmr:: class below.
    //
    Geom_info,// Dim_int, Enum_int, Local_int vals
    Phys_info,// Combine with Geom_info as Part_info?
#endif
  end};// The last item is "end" to indicate number of enumerated values
#if 1
  static const std::vector<fmr::Vals_type> vals_type = {
    fmr::Vals_type::None,
    fmr::Vals_type::Error,
    fmr::Vals_type::Unknown,
#include "vals-type.inc"
    fmr::Vals_type::Local,
    fmr::Vals_type::Local,
    fmr::Vals_type::None
  };
  static const std::vector<std::string> vals_name = {
    "data_none",
    "data_err",
    "data_unk",
#include "vals-name.inc"
    "Geom_info",
    "Phys_info",
    "END"
  };
  static const std::vector<std::string> vals_info = {
    "no data",
    "data error",
    "unknown data",
#include "vals-info.inc"
    "geometry info (Dim_int, Enum_int, Local_int vals)",
    "physics info (Dim_int, Enum_int, Local_int vals)",
    "Data enum end marker"
  };
#else
  typedef std::tuple<fmr::Vals_type,std::string,std::string> Name_pair;
  static const std::map<Data,Name_pair> Data_name {//TODO lowercase these maps?
    {Data:: None        , Name_pair {fmr::Vals_type::None,
           "data_none"  ,"no data"}},
    {Data:: Error       , Name_pair {fmr::Vals_type::Error,
           "data_err"   ,"data error"}},
    {Data:: Unknown     , Name_pair {fmr::Vals_type::Unknown,
           "data_unk"   ,"unknown data"}},
    {Data:: Gset_n      , Name_pair {fmr::Vals_type::Local,
           "gset_n"     ,"number of geometry sets"}},
    {Data:: Geom_d      , Name_pair {fmr::Vals_type::Dim,
           "geom_d"     ,"enclosing space dimension"}},
    {Data:: Sims_size   , Name_pair {fmr::Vals_type::Enum,
           "sims_size"  ,"sim size type (enum)"}},
    {Data:: Part_type   , Name_pair {fmr::Vals_type::Enum,
           "part_type"  ,"partitioning algorithm (enum)"}},
    {Data:: Part_n      , Name_pair {fmr::Vals_type::Local,
           "part_n"     ,"number of partititions"}},
    {Data:: Geom_n      , Name_pair {fmr::Vals_type::Local,
           "geom_n"     ,"number of meshes + grids"}},
    {Data:: Node_n      , Name_pair {fmr::Vals_type::Local,
           "node_n"     ,"number of nodes"}},
    {Data:: Node_x      , Name_pair {fmr::Vals_type::Geom,
           "node_x"     ,"node x-ordinates"}},
    {Data:: Node_y      , Name_pair {fmr::Vals_type::Geom,
           "node_y"     ,"node y-ordinates"}},
    {Data:: Node_z      , Name_pair {fmr::Vals_type::Geom,
           "node_z"     ,"node z-ordinates"}},
    {Data:: Node_coor   , Name_pair {fmr::Vals_type::Geom,
           "node_coor"  ,"node coordinates"}},
    {Data:: Rots_type   , Name_pair {fmr::Vals_type::Geom,
           "rots_type"  ,"Orientation type"}},
    {Data:: Axis_rots   , Name_pair {fmr::Vals_type::Geom,
           "axis_rots"  ,"Euler angle(s)"}},
    {Data:: Axis_quat   , Name_pair {fmr::Vals_type::Geom,
           "axis_quat"  ,"unit quaternian orientation"}},
    {Data:: Mesh_n      , Name_pair {fmr::Vals_type::Local,
           "mesh_n"     ,"number of unstructured meshes"}},
    {Data:: Elem_n      , Name_pair {fmr::Vals_type::Local,
           "elem_n"     ,"number of elements"}},
    {Data:: Elem_type   , Name_pair {fmr::Vals_type::Enum,
           "elem_type"  ,"element type"}},
    {Data:: Elem_node_n , Name_pair {fmr::Vals_type::Local,
           "elem_node_n","number of nodes / elem"}},// vrtx+edge+face+body nodes
    {Data:: Elem_jacs_n , Name_pair {fmr::Vals_type::Local,
           "elem_jacs_n","number of jacobians / elem"}},
    {Data:: Jacs_type   , Name_pair {fmr::Vals_type::Enum
          ,"jacs_type"  ,"jacobian type"}},
    {Data:: Jacs_size   , Name_pair {fmr::Vals_type::Dim,
           "jacs_size"  ,"element jacobians and determinants"}},
    {Data:: Jacs_dets   , Name_pair {fmr::Vals_type::Geom,
           "jacs_dets"  ,"element jacobian and determinant values"}},
    {Data:: Node_id     , Name_pair {fmr::Vals_type::Global,
           "node_id"    ,"node global id"}},
    {Data:: Elem_id     , Name_pair {fmr::Vals_type::Global,
           "elem_id"    ,"element global id"}},
    //NOTE must list ALL enum items consecutively or .at lookup might segfault
//    {Data:: Grid_n      , Name_pair{"grid_n"   ,"number of structured grids"}},
    {Data:: end, Name_pair{fmr::Vals_type::None,"END","Data enum end marker"}}
  };
#endif
#if 1
  //TODO Remove these heterogeneous data types. Add back if sync is needed.
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
  inline Elid_int make_elid (//TODO Remove?
    Elem e, fmr::math::Poly y, fmr::Dim_int p) {
    constexpr auto ysz = 8*sizeof(y), psz = 8*sizeof(p);
    return
      ( (Elid_int(fmr::enum2val(e)) << (ysz+psz))
      + (Elid_int(fmr::enum2val(y)) << (psz)) + p );
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
