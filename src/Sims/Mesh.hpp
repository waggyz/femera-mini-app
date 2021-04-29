#ifndef FMR_HAS_MESH_HPP
#define FMR_HAS_MESH_HPP
/** */

#include <unordered_map>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
class Mesh : public Geom {// Pure virtual? Mesh?
  // typedefs ---------------------------------------------------------------
  // member variables -------------------------------------------------------
  // this->task may contain child meshes
  public:
  protected:
  private:
    // Finite element method variables
#if 0
    // Unstructured meshes need several lookup tables.
    std::unordered_map<fmr::Global_int, fmr::Local_int> local_node_id ={};
    std::unordered_map<fmr::Global_int, fmr::Local_int> local_elem_id ={};
    //
    fmr::Global_int_vals elem_id// of each element in this part
      = fmr::Global_int_vals (fmr::Data::Elem_id, fmr::math::Natural);
    //
    fmr::Global_int_vals node_id// of each node in this part
      = fmr::Global_int_vals (fmr::Data::Node_id, fmr::math::Natural);
    //
//    fmr::Local_int_vals  conn//Elem_conn -> e.g. Conn_tet4
//      = fmr::Local_int_vals  (fmr::Data::Elem_conn, fmr::math::Natural);
    //
    fmr::Geom_float_vals jacs// element jacobian data (inverted, may have det)
      = fmr::Geom_float_vals (fmr::Data::Jacs_dets, fmr::math::Real);
    //
    fmr::Geom_float_vals node
      = fmr::Geom_float_vals (fmr::Data::Node_coor, fmr::math::Real);
#endif
    std::map<fmr::Data,fmr::Geom_float_vals> geoms ={};
    //
    fmr::Local_int        node_n = 0;
    fmr::Local_int        elem_n = 0;
    fmr::Elem_info     elem_info = fmr::Elem_info ();//TODO replace with Elem ?
    fmr::math::Zomplex math_type = fmr::math::Real;
  // methods ----------------------------------------------------------------
  public:
    int prep () final override;
    int chck () final override;
    int run  () final override;
    //fmr::Geom_float get_geom_val (const fmr::Data, const size_t ix);
    int ini_data_vals (const Data_list, const size_t n) final override;
    int get_data_vals (const fmr::Data_id, const Data_list) final override;
    //
    fmr::Local_int make_jacs ();// Returns number of nonpositive determinants
  protected:
#if 0
    int prep () override;
    int init_task (int* argc, char** argv) final override;
    int exit_task (int  err) final override;
#endif
  private:
  public:
    virtual ~Mesh  ()      noexcept=default;
    Mesh           (Sims*) noexcept;
    Mesh           ()              =delete;
    Mesh           (Mesh const&)   =delete;// not copyable
    Mesh operator= (const Mesh&)   =delete;
};
}//end Femera namespace
namespace fmr { namespace elem {//FIXME move to elem.hpp
  template<typename T>
  fmr::Local_int make_jacobian (T* FMR_RESTRICT jacs,//TODO T jacs FMR_RESTRICT &
    const fmr::Local_int conn_n, const fmr::Local_int* FMR_RESTRICT conn,
    const T* FMR_RESTRICT shpg,
    const T* FMR_RESTRICT x, const T* FMR_RESTRICT y, const T* FMR_RESTRICT z) {
    const fmr::Local_int jacs_sz = 10;//TODO = fmr::elem::jac3_size
    jacs [jacs_sz-1] = T (0.0);
    FMR_PRAGMA_OMP_SIMD
    for (fmr::Local_int i=0; i<conn_n; i++) {
      jacs [jacs_sz-1]//TODO Fix fake det calc.
        += x [conn[i]] * shpg[3* i+0]
        +  y [conn[i]] * shpg[3* i+1]
        +  z [conn[i]] * shpg[3* i+2];
    }
    jacs [jacs_sz-1] /= T (3*4);
    return jacs [jacs_sz-1] <= T (0.0);
  }
} }//end fmr::elem:: namespace
#else
//end FMR_HAS_MESH_HPP
#endif
