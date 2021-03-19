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
    fmr::Dim_int mesh_d = 0;
    fmr::Dim_int elem_d = 0;
    //
    fmr::Dim_int elem_conn_n = 0;
//    fmr::Dim_int elem_jacs_n = 0;
    //
    // Unstructured meshes need a lot of lookup tables.
    std::unordered_map<fmr::Global_int, fmr::Local_int> local_node_id ={};
    std::unordered_map<fmr::Global_int, fmr::Local_int> local_elem_id ={};
    //
    fmr::Global_int_vals global_elem_id =// of each element in this part
    fmr::Global_int_vals (fmr::Data::Elem_id, fmr::math::Natural);
    //
    fmr::Global_int_vals global_node_id =// of each node in this part
    fmr::Global_int_vals (fmr::Data::Node_id, fmr::math::Natural);
    //
    fmr::Local_int_vals  conn =//Elem_conn -> e.g. Conn_tet4
    fmr::Local_int_vals  (fmr::Data::Elem_conn, fmr::math::Natural);
    //
    fmr::Geom_float_vals jacs =// element jacobian data (inverted, may have det)
    fmr::Geom_float_vals (fmr::Data::Jacs_dets, fmr::math::Real);
    //
    fmr::Geom_float_vals node =
    fmr::Geom_float_vals (fmr::Data::Node_coor, fmr::math::Real);
  // methods ----------------------------------------------------------------
  public:
    int chck () override;
    int run  () override;
  protected:
#if 0
    int prep () override;
    int init_task (int* argc, char** argv) final override;
    int exit_task (int  err) final override;
#endif
  private:
  public:
    virtual ~Mesh () noexcept=default;// Virtual destructor for virtual class
    Mesh (Sims*) noexcept;
    Mesh ()=delete;
    Mesh (Mesh const&)=delete;// not copyable
    Mesh operator= (const Mesh&)=delete;
};
}//end Femera namespace
#else
//end FMR_HAS_MESH_HPP
#endif
