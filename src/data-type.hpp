#ifndef FMR_HAS_DATATYPE_HPP
#define FMR_HAS_DATATYPE_HPP

#include "math.hpp"

#include <valarray>
#include <map>
#include <unordered_map>
#include <functional>        // std::hash

namespace fmr {//TODO? namespace data {
  /* These typedefs, structs, and static stuff are used throughout
   * fmr:: and Femera:: namespaces.
   * */
  typedef std::string Data_id;
  typedef std::vector<fmr::Local_int> Tree_path;
  enum class Tree_type : Enum_int {None=0, Error, Unknown,
    Mtrl, Sims, Gset, Part, Join_part, Mesh, Grid,
  end};
  struct Math_size {// Size and algebra type of data.
    size_t           size = 0;
    math::Zomplex algebra = math::Real;
    // constructor
    Math_size (const math::Zomplex z, const size_t sz) : size(sz), algebra(z){}
  };
  typedef std::unordered_map<Global_int,Local_int> Node_local_map;
  typedef std::unordered_map<Global_int,Local_int> Elem_local_map;
  enum class Data : Enum_int {None=0, Error, Unknown,
    // Homogeneous data types
    //
    Sims_type, Time_type,            // Enum_int
    Sims_n,                          // Local_int
    //
    Phys_d,                          // Dim_int
    Phys_type,                       // Enum_int [Mtrl_n]
    Mtrl_n,                          // Local_int
    //
    Geom_d,                          // Dim_int
    Gset_n,                          // Local_int
    //
    Part_n,                          // Local_int
    Part_conn_n,                     // Local_int: shared surfaces
    Mesh_conn_n,                     // Local_int [Part_n]: shared surfaces
    //
    // Unstructured (finite element) mesh
    Mesh_n,                          // Local_int
    Elem_d, Elem_jacs_sz, Elem_pord, // Dim_int    [Mesh_n]
    Jacs_type, Elem_type, Elem_poly, // Enum_int   [Mesh_n]
    Elem_node_n, Elem_jacs_n,        // Local_int  [Mesh_n]
    Elem_n, Node_n,                  // Local_int  [Mesh_n]
    Elem_conn,                       // Local_int  [Elem_n,Elem_node_n]
    // Read as Global_ids and build Global_id <=> Local_id maps while reading.
    Node_local_id,                   // Local_int  [Node_n]
    Elem_local_id,                   // Local_int  [Elem_n]
    Node_global_id,                  // Global_int [Node_n]
    Elem_global_id,                  // Global_int [Elem_n]
    //
    // Structured grid
    Grid_conn_n,                     // Local_int  [Part_n]: shared grid surfs
    Grid_n,                          // Local_int
    Cell_d,                          // Dim_int    [Grid_n]
    Cell_type,                       // Enum_int   [Grid_n]
    Grid_size,                       // Global_int [Cell_d]
    //
    Mesh_dirs,             // Geom_float [Mesh_n,4] unit quaternion orientation
    Node_x, Node_y, Node_z,// Geom_float [Node_n]
    Node_coor,             // Geom_float [Node_n,Elem_d]
    Jacs_dets,             // Geom_float [Elem_n,Elem_jacs_n,Elem_jacs_sz]
#if 1
    //TODO Remove heterogeneous data types. Add back if sync or distribution
    //     is needed.
    // Heterogenous data types: packed as Enum_int, Local_int, or Global_int.
    // The enum items have a corresponding fmr:: class below.
    //
    Elem_ig, Node_ig,
    Geom_info,// Dim_int, Enum_int, Local_int vals
    Phys_info,//TODO Combine with Geom_info as Part_info?
#endif
    //
  end};// The last item is "end" to indicate size
  typedef std::pair<std::string,std::string> Short_name;
  static const std::map<Data,Short_name> Data_name {
    {Data:: None     , Short_name{"data_none","no data"}},
    {Data:: Error    , Short_name{"data_err" ,"data error"}},
    {Data:: Unknown  , Short_name{"data_unk" ,"unknown data"}},
    {Data:: Node_x   , Short_name{"node_x"   ,"node x-ordinates"}},// Geom_float
    {Data:: Node_y   , Short_name{"node_y"   ,"node y-ordinates"}},// Geom_float
    {Data:: Node_z   , Short_name{"node_z"   ,"node z-ordinates"}},// Geom_float
    {Data:: Node_coor, Short_name{"node_coor","node coordinates"}},// Geom_float
    {Data:: Jacs_dets, Short_name{
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
    Part_conn_n, Mesh_conn_n, Grid_conn_n,// Local_int
    Geom_d,                               // Dim_int
  end};
  enum class Phys_info : Enum_int {isok=0,
    Mtrl_n,  // Local_int //TODO here?
    Sim_time,// Enum_int
    Phys_d,  // Dim_int
  end};
  //NOTE For compile-time lookup use Data_size.at(type),
  //     NOT Data_size[type] nor Data_size.find(type).
  static const std::map<Data, Math_size> Data_size {//TODO Remove.
    // for heterogeneous data
    {Data::Geom_info, Math_size(math::Natural, enum2val (Geom_info::end))},
    {Data::Phys_info, Math_size(math::Natural, enum2val (Phys_info::end))}
  };
#endif
#if 0
    //TODO Remove heterogenous data types. Add back if sync is needed.
  /* Almost no info is stored at the sims collection level in CGNS and Gmsh
   * files. (But, a Gmsh parameterized .geo file could be considered a
   * collection.) Typical single-file formats store a lot of info at individual
   * sim level. The coll/sim/part/mesh/etc. level is encoded in the data_id
   * associated (as the key of a Data->Data_*_vals map) with each Vals
   * instance.
   *
   * Sims_size, Sims_info, Geom_size may be stored at root, sim, part, mesh, or
   * grid level in a file hierarchy. They are populated by scan_file_data(..)
   * and read_sims_info(..).//TODO or e.g. read_/get_data(..,Data::Sims_info)
   *                                       read_/get_data(..,Data::Geom_size)
   * Some of the data in the structures below are redundant, or can be
   * calculated from other values. In most cases, only some of the fields will
   * be populated, which fields depend on the associated data_id (e.g. a
   * Part's Sims_size will be zero, and usually (but not always) have only one
   * non-zero Part_n, Mesh_n, or Grid_n children.
   */
  enum class Sims_size : Enum_int {isok=0, // Pack as Local_int. Read first.
    Mtrl_n, Gset_n, Part_n, Mesh_n, Grid_n,// Local_int scalar
    Part_conn_n, Mesh_conn_n, Grid_conn_n, // Local_int scalar: shared surfaces
  end};
  enum class Sims_info : Enum_int {isok=0, // Pack as Enum_int.
    Phys_d, Geom_d,                        // Dim_int  scalar
    Sims_type, Time_type,                  // Enum_int scalar
    Elem_d,                                // Dim_int  [Mesh_n]
    Cell_d,                                // Dim_int  [Cell_n]
    Phys_type,                             // Enum_int [Mtrl_n]
    Cell_type,                             // Enum_int [Grid_n]
    Jacs_type, Elem_type, Elem_poly,       // Enum_int [Mesh_n]
    Elem_pord,                             // Dim_int  [Mesh_n]
    Elem_node_n, Elem_jacs_n,              // Enum_int [Mesh_n]
  end};
  enum class Geom_size : Enum_int {isok=0, // Pack as Global_int.
    Node_n, Elem_n, Elem_conn_n,           // Global_int scalar
    Node_gd,                               // Global_int [Node_n]
    Elem_gd,                               // Global_int [Elem_n]
    Elem_conn,                             // Global_int [Elem_conn_n] *
    Grid_dims,                             // Global_int [Cell_d] **
    //  * Elem_conn_n = Elem_n * Elem_node_n
    // ** Cell_d csn be inferred from data.size().
  end};
#endif
  //
  enum class Elem : Enum_int {None=0, Error, Unknown,
    Bars,    // Beam,
    Tris,    // Quad,
    Tets,    // Pymd, Prsm, Cube,
    // Mbrn, Shll_xxxx
    // Cell_FD, Cell_FV,
    // FD1D, FD2D, FD3D, FV1D, FV2D, FV3D, SG1D, SG2D, SG3D, // grid cell types
  end};
#if 0
  //TODO Change to std::array<Math_size,enum2val(Data::end)>?
  static const std::array<Math_size,enum2val(Data::end)> Data_size_test {
    this[enum2val (Data::Geom_info)]//TODO Does not work.
      = Math_size(math::Natural, enum2val (Geom_info::end)),
    this[enum2val (Data::Phys_info)]
      = Math_size(math::Natural, enum2val (Phys_info::end))
  };
#endif
  inline Elem_id_int make_elem_id (Elem e, fmr::math::Poly y, fmr::Dim_int p){
    constexpr auto ysz = 8*sizeof(y), psz = 8*sizeof(p);
    return
      ( (Elem_id_int(fmr::enum2val(e)) << (ysz+psz))
      + (Elem_id_int(fmr::enum2val(y)) << (psz)) + p );
  }
#if 0
  std::size_t hash_elem_id (Elem e, Poly y, fmr::Dim_int p){std::size_t id = 0;
    constexpr int esz = sizeof(e), ysz = sizeof(y), psz = sizeof(p);
    if ((esz + ysz + psz) <= sizeof(std::size_t) ){
      id = std::hash<std::size_t>{}
        ( (std::size_t(fmr::enum2val(e)) << (ysz+psz))
        + (std::size_t(fmr::enum2val(y)) << (psz))
        + p );
    }else{
      id = std::hash<std::string>{} (
        std::to_string()+":"+std::to_string()+":"+std::to_str());
    }
    return id;
  }
#endif
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
