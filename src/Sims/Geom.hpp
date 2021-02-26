#ifndef FMR_HAS_GEOM_HPP
#define FMR_HAS_GEOM_HPP
/** */

#include <unordered_map>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
class Geom : public Sims {// Pure virtual?
  // typedefs ---------------------------------------------------------------
  // member variables -------------------------------------------------------
  public:
  protected:
    fmr::Dim_int      geom_d = 0;
  private:
#if 0
    fmr::Local_int     halo_part_n = 0;//TODO Move to Halo (derived from Sync)
    fmr::Local_int     halo_elem_n = 0;
    fmr::Local_int ext_halo_node_n = 0;
    fmr::Local_int  my_halo_node_n = 0;
#endif
  // methods ----------------------------------------------------------------
  public:
    int chck () override;
    int run  () override;
#if 0
    fmr::Global_int  get_elem_n (std::string model fmr::Part_id);// also cells
    fmr::Global_int  get_node_n ();
    fmr::Global_int  get_part_n ();
#endif
  protected:
    int get_geom_n ();// lower-level meshes and grids
#if 0
    int prep () override;
    int init_task (int* argc, char** argv) final override;
    int exit_task (int  err) final override;
#endif
  private:
  protected:
    Geom ()=default;// called implicitly by child constructors
  public:
    virtual ~Geom () noexcept=default;// Virtual destructor for virtual class
    Geom (Sims*) noexcept;
    Geom (Geom const&)=delete;// not copyable
    Geom operator= (const Geom&)=delete;
};
}//end Femera namespace
#else
//end FMR_HAS_GEOM_HPP
#endif
