#ifndef FMR_HAS_DATATYPE_HPP
#define FMR_HAS_DATATYPE_HPP

#include "math.hpp"

#include <valarray>
#include <map>
#include <unordered_map>
#include <array>
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
    // included content is from data/vals-type.csv using tools/vals-type.sh
#include "data-enum.inc"
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
  end};// The last item is "end" to indicate the number of enumerated values.
  static const std::array<Vals_type,enum2val(Data::end)+1> vals_type ={
    fmr::Vals_type::None,
    fmr::Vals_type::Error,
    fmr::Vals_type::Unknown,
#include "vals-type.inc"
    fmr::Vals_type::Local,
    fmr::Vals_type::Local,
    fmr::Vals_type::None
  };
  static const std::array<std::string,enum2val(Data::end)+1> vals_name ={
    "data_none",
    "data_err",
    "data_unk",
#include "vals-name.inc"
    "Geom_info",//TODO Remove these heterogeneous data types.
    "Phys_info",//     Add back if sync is needed.
    "END"
  };
  static const std::array<std::string,enum2val(Data::end)+1> vals_info ={
    "no data",
    "data error",
    "unknown data",
#include "vals-info.inc"
    "geometry info (Dim_int, Enum_int, Local_int) vals",
    "physics info (Dim_int, Enum_int, Local_int) vals",
    "Data enum end marker"
  };
#if 1
  //TODO Remove these heterogeneous data types. Add back if needed.
  /* Human-readable (enum) index names for heterogenous data types above
   * isok: for packing a bit array indicating which items have valid data.
   * The last item is always "end" to indicate the count of enum items for the
   * Data_size map below.
  */
  enum class Geom_info : Enum_int {isok=0, // Use for Sims and Part data.
    Gset_n, Part_n, Gcad_n, Mesh_n, Grid_n,// Local_int
    Part_halo_n, Mesh_halo_n, Grid_halo_n, // Local_int
    Geom_d,                                // Dim_int
  end};
  enum class Phys_info : Enum_int {isok=0,
    Mtrl_n,  // Local_int //TODO here?
    Sim_time,// Enum_int
    Phys_d,  // Dim_int
  end};
  struct Math_size {// Size and algebra type of data elements.
    size_t           size = 0;
    math::Zomplex algebra = math::Real;
    // constructor
    Math_size (const math::Zomplex z, const size_t sz) : size(sz), algebra(z){}
  };
  //NOTE For compile-time lookup use Data_size.at(type),
  //     NOT Data_size[type] nor Data_size.find(type).
  static const std::map<Data,Math_size> Data_size {//TODO Remove.
    // for heterogeneous data
    {Data::Geom_info, Math_size(math::Natural, enum2val (Geom_info::end))},
    {Data::Phys_info, Math_size(math::Natural, enum2val (Phys_info::end))}
  };
#endif
  enum class Elem_form : Enum_int {None=0, Error, Unknown,//TODO also Cell_form?
    User,
    Node,                  // 0D
    Line,                  // 1D
    Tris, Quad,            // 2D
    Tets, Prmd, Prsm, Cube,// 3D
  end};
  static const std::array<std::string,enum2val(Elem_form::end)+1>
    elem_form_name ={
    "no element shape",
    "element shape error",
    "unknown element shape",
    "user-defined element shape",
    //
    "node",
    "line",
    "tris","quad",
    "tets","pyramid","prism","cube",
    //
    "Elem_form end marker"
  };
  static const std::array<fmr::Dim_int,enum2val(Elem_form::end)+1>
    elem_form_d ={
    0,//"no element shape",
    0,//"element shape error",
    0,//"unknown element shape",
    0,//"user-defined element shape",
    //
    0,// Node,
    1,// Line,
    2,2,// Tris, Quad,
    3,3,3,3,// Tets, Prmd, Prsm, Cube,
    //
    0 //"Elem_form end marker"
  };
  struct Elem_info {
    Local_int       node_n = 0;
    Local_int  elem_jacs_n = 1;//TODO jacs/elem
    Elem_form       form   = Elem_form::Unknown;
    math::Poly      poly   = math::Poly::Unknown;
    Dim_int         pord   = 0;
    Dim_int         elem_d = 0;
    Dim_int         vert_n = 0;
#if 0
    Dim_int         edge_n = 0;
    Dim_int         face_n = 0;
    Dim_int         volu_n = 1;
#endif
    Dim_int each_jacs_size =10;//TODO size of each jacs (4,10)
    Elem_info () {}
    Elem_info (const Elem_form f, const math::Poly y, const Dim_int p)
      : form (f), poly (y), pord (p) {
      elem_d = elem_form_d [enum2val (f)];
      node_n = math::poly_terms (elem_d, y, p);
    }
    Elem_info (const Elem_form f, const Local_int n) : form (f) {
      switch (f) {
        case Elem_form::Line :
          vert_n = 2;
          switch (n) {
            case  1 : pord = 1; poly = math::Poly::Full; break;
            case  2 : pord = 2; poly = math::Poly::Full; break;
            case  3 : pord = 3; poly = math::Poly::Full; break;
          default : {}
        } break;
        case Elem_form::Tris :
          vert_n = 3;
          switch (n) {
            case  3 : pord = 1; poly = math::Poly::Full; break;
            case  6 : pord = 2; poly = math::Poly::Full; break;
            case  9 : pord = 3; poly = math::Poly::Full; break;
          default : {}
        } break;
        case Elem_form::Tets :
          vert_n = 4;
          switch (n) {
            case  4 : pord = 1; poly = math::Poly::Full; break;
            case 10 : pord = 2; poly = math::Poly::Full; break;
            case 20 : pord = 3; poly = math::Poly::Full; break;
          default : {}
        } break;
        default : {}
      }
      elem_d = elem_form_d [enum2val (f)];
      node_n = fmr::math::poly_terms (elem_d, poly, pord);//TODO WARN if !=n
    }
    Elem_info            (Elem_info const&) =default;// copyable
    Elem_info& operator= (const Elem_info&) =default;
  };
#if 1
  enum class Elem : Enum_int {None=0, Error, Unknown,//TODO Replace Elem_form?
    Node,
    Line,    // Beam
    Tris,    // Quad,
    Tets,    // Pyrm, Prsm, Cube,
    // Mbrn, Shll_xxxx
    // Cell_FD, Cell_FV,
    // FD1D, FD2D, FD3D, FV1D, FV2D, FV3D, SG1D, SG2D, SG3D, // grid cell types
  end};
  inline Elid_int make_elid (//TODO Remove?
    const Elem e, const fmr::math::Poly y, const fmr::Dim_int p) {
    constexpr auto ysz = 8*sizeof(y), psz = 8*sizeof(p);
    return
      ( (Elid_int(fmr::enum2val(e)) << (psz+ysz))
      + (Elid_int(fmr::enum2val(y)) << (psz))
      + p );
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
