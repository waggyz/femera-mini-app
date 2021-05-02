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
  namespace elem {
    static const std::vector<fmr::Dim_int> jacs_size = {1,1,4,10};
  }// end fmr::elem:: namespace
  struct Elem_info {
    Local_int  each_node_n = 0;// nodes/elem
    Local_int  each_jacs_n = 0;//  jacs/elem
    Geom_float elem_size   = Geom_float (0.0);// natural length/area/volume
    Elem_form  elem_form   = Elem_form::Unknown;
    math::Poly elem_poly   = math::Poly::Unknown;
    math::Poly jacs_poly   = math::Poly::Unknown;
    Dim_int         jacs_p = 0;
    Dim_int         elem_p = 0;
    Dim_int         elem_d = 0;
    Dim_int         vert_n = 0;//TODO needed or function of form?
#if 0
    Dim_int         edge_n = 0;//TODO needed or function of form?
    Dim_int         face_n = 0;
    Dim_int         volu_n = 1;
#endif
    Dim_int each_jacs_size = 0;
    Elem_info () {}
    Elem_info (const Elem_form f, const math::Poly y, const Dim_int p)
      : elem_form (f), elem_poly (y), elem_p (p) {
      // Construct from shape and interpolating polynomial.
      elem_d         = elem_form_d [enum2val (f)];
      each_node_n    = math::poly_terms (y, elem_d, p);
      jacs_poly      = elem_poly;
      each_jacs_size = fmr::elem::jacs_size [elem_d];
      switch (f) {// Set element jacobian order and number/element.
        //TODO Elem_form:: Teta, Tria, Lina : Affine tets/tris/line?
        case Elem_form::Line :// constant jacs when side nodes at
        case Elem_form::Tris :// natural locations
        case Elem_form::Tets : jacs_p = 0; each_jacs_n = 1; break;
        default : jacs_p = (elem_p > 0) ? fmr::Dim_int (elem_p - 1) : 0;
          each_jacs_n= fmr::math::poly_terms (math::Poly::Full, elem_d, jacs_p);
    } }
    Elem_info (const Elem_form f, const Local_int n) :
      each_node_n (n), elem_form (f) {// Construct from shape & number of nodes.
      switch (f) {
        case Elem_form::Line :
          elem_size = fmr::Geom_float (1.0);
          vert_n = 2; jacs_p = 0; each_jacs_n = 1;
          switch (n) {// constant jacs when side nodes at natural locations
            case  1 : elem_p = 1; elem_poly = math::Poly::Full; break;
            case  2 : elem_p = 2; elem_poly = math::Poly::Full; break;
            case  3 : elem_p = 3; elem_poly = math::Poly::Full; break;
          default: {}
        } break;
        case Elem_form::Tris :
          elem_size = fmr::Geom_float (1.0/2.0);
          vert_n = 3; jacs_p = 0; each_jacs_n = 1;
          switch (n) {// constant jacs when side nodes at natural locations
            case  3 : elem_p = 1; elem_poly = math::Poly::Full; break;
            case  6 : elem_p = 2; elem_poly = math::Poly::Full; break;
            case  9 : elem_p = 3; elem_poly = math::Poly::Full; break;
          default : {}
        } break;
        case Elem_form::Tets :
          elem_size = fmr::Geom_float (1.0/6.0);
          vert_n = 4; jacs_p = 0; each_jacs_n = 1;
          switch (n) {// constant jacs when side nodes at natural locations
            case  4 : elem_p = 1; elem_poly = math::Poly::Full; break;
            case 10 : elem_p = 2; elem_poly = math::Poly::Full; break;
            case 20 : elem_p = 3; elem_poly = math::Poly::Full; break;
          default : {}
        } break;
        default : jacs_p = (elem_p > 0) ? fmr::Dim_int (elem_p - 1) : 0;
          each_jacs_n= fmr::math::poly_terms (math::Poly::Full, elem_d, jacs_p);
      }
      elem_d      = elem_form_d [enum2val (f)];
      each_node_n = fmr::math::poly_terms (elem_poly, elem_d, elem_p);
      //TODO        WARN if each_node_n != n
      jacs_poly   = elem_poly;
      each_jacs_size = fmr::elem::jacs_size [elem_d];
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
