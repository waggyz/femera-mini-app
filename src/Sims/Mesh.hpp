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
    //
    fmr::Geom_float volume = fmr::Geom_float(0.0);//TODO Remove
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
namespace fmr {
  namespace math {//TODO move to math.hpp
  template<typename T>
  inline T det3 (const T* m) {// m is 3x3
    // FLOPS: 4*3 + 1 = 12
    return(
        m[3* 0+0] * (m[3* 1+1] * m[3* 2+2] - m[3* 2+1] * m[3* 1+2])
      + m[3* 0+1] * (m[3* 1+2] * m[3* 2+0] - m[3* 2+2] * m[3* 1+0])
      + m[3* 0+2] * (m[3* 1+0] * m[3* 2+1] - m[3* 2+0] * m[3* 1+1]) );
  }
  template<typename T>
  inline int inv3 (T* m, const T det) {// returns 3x3 inverse in m
    // FLOPS: 4*9 + 1 = 37
#if 0
    if (det==0) {return 1;}//TODO
#endif
    const T dinv = T(1.0) / det;
    FMR_PRAGMA_OMP_SIMD
    T minv[9]; for (int i=0; i<9; i++) {minv[i] = dinv;};
    //
    minv[3* 0+0]*= (m[3* 1+1] * m[3* 2+2] - m[3* 2+1] * m[3* 1+2]);
    minv[3* 0+1]*= (m[3* 0+2] * m[3* 2+1] - m[3* 0+1] * m[3* 2+2]);
    minv[3* 0+2]*= (m[3* 0+1] * m[3* 1+2] - m[3* 0+2] * m[3* 1+1]);
    //
    minv[3* 1+0]*= (m[3* 1+2] * m[3* 2+0] - m[3* 1+0] * m[3* 2+2]);
    minv[3* 1+1]*= (m[3* 0+0] * m[3* 2+2] - m[3* 0+2] * m[3* 2+0]);
    minv[3* 1+2]*= (m[3* 1+0] * m[3* 0+2] - m[3* 0+0] * m[3* 1+2]);
    //
    minv[3* 2+0]*= (m[3* 1+0] * m[3* 2+1] - m[3* 2+0] * m[3* 1+1]);
    minv[3* 2+1]*= (m[3* 2+0] * m[3* 0+1] - m[3* 0+0] * m[3* 2+1]);
    minv[3* 2+2]*= (m[3* 0+0] * m[3* 1+1] - m[3* 1+0] * m[3* 0+1]);
    FMR_PRAGMA_OMP_SIMD
    for (int i=0; i<9; i++) {m[i]=minv[i];};//TODO memcpy
    return (det < 0) ? -1 : 0;
  }
  template<typename T>
  inline int perf_inv3  (fmr::perf::Meter* time, const fmr::Local_int elem_n,
    const T* m) {
    time->flops += elem_n * 37;
    time->bytes += elem_n *(10 +9) * sizeof (m);// read + write
    return 0;
  }
  }//end fmr::math:: namespace
  namespace elem {//TODO move to elem.hpp
  template<typename T>
  inline fmr::Local_int make_inv3_jacdet (T* FMR_RESTRICT jacs,
    const fmr::Local_int conn_n, const fmr::Local_int* FMR_RESTRICT conn,
    const T* FMR_RESTRICT shpg,//TODO e.g. T jacs FMR_RESTRICT &
    const T* FMR_RESTRICT x, const T* FMR_RESTRICT y, const T* FMR_RESTRICT z) {
    const auto jacs_sz = fmr::elem::jacs_size [3];//TODO Only 3D
    FMR_PRAGMA_OMP_SIMD
    for (fmr::Local_int i=0; i<jacs_sz; i++) {jacs [i] = T (0.0);}
    //TODO_FMR_PRAGMA_OMP_SIMD
    for (fmr::Local_int j=0; j<conn_n; j++) {//TODO permute loops?
    for (fmr::Local_int i=0; i<3; i++) {
      jacs [3* i+0] += shpg [3* j+i] * x [conn[j]];//TODO transpose shpg?
      jacs [3* i+1] += shpg [3* j+i] * y [conn[j]];
      jacs [3* i+2] += shpg [3* j+i] * z [conn[j]];
    } }// 2* 3*3*conn_n FLOP
    jacs [jacs_sz-1] = fmr::math::det3 (jacs);// determinant 12 FLOP
    fmr::math::inv3 (jacs, jacs [jacs_sz-1]);// 37 FLOP
    return jacs [jacs_sz-1] <= T (0.0);
  }
  template<typename T>
  inline int perf_jacobian (fmr::perf::Meter* time, const fmr::Local_int elem_n,
    const T* FMR_RESTRICT jacs,
    const fmr::Local_int conn_n, const fmr::Local_int* FMR_RESTRICT conn,
    const T* FMR_RESTRICT,
    const T* FMR_RESTRICT x, const T* FMR_RESTRICT, const T* FMR_RESTRICT) {
    const auto jacs_sz = fmr::elem::jacs_size [3];//TODO Only 3D
    time->flops += elem_n * (2*3*3*conn_n + 37+12) + 1;
    time->bytes += elem_n * (
      3 * conn_n  * sizeof (   x[0])  // read
      +   conn_n  * sizeof (conn[0])  // read
      +   jacs_sz * sizeof (jacs[0]));// write
    return 0;
  }
} }//end fmr::elem:: namespace
#else
//end FMR_HAS_MESH_HPP
#endif
