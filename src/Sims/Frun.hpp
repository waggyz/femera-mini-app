#ifndef FMR_HAS_FRUN_HPP
#define FMR_HAS_FRUN_HPP
/** */

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
class Frun final : public Sims {//TODO Problably NOT derived from Sims or Work.
  //TODO Change to Exec? Algo?
  /* This simulation scheduler runs all of the sims that have been pre-loaded
   * into its task stack. It does no load balancing.
   * */
  // typedefs ---------------------------------------------------------------
  // member variables -------------------------------------------------------
  public:
  protected:
  private:
    fmr::Local_int sims_ix = 0;// sim number
    fmr::Dim_int   sims_lv = 1;// subsim hierarchy level
#if 0
    fmr::Local_int gset_n =  0;
    fmr::Local_int part_n =  0;
    fmr::Local_int conn_n =  0;
    //
    fmr::Local_int mesh_n =  0;//TODO move to Part?
    fmr::Local_int grid_n =  0;//TODO move to Part?
    //
    fmr::Local_int mtrl_n =  0;
    //
    fmr::Sim_time sim_time = fmr::Sim_time::Unknown;
    //
    fmr::Dim_int   geom_d =  0;
    fmr::Dim_int   phys_d =  0;
#endif
  // methods ----------------------------------------------------------------
  public:
#if 0
    int get_gset_n ();//TODO move these to Part?
    int get (const fmr::Geom_info, const fmr::Local_int &){return 1;}
    int get (const fmr::Geom_info, const fmr::Sim_time  &){return 1;}
    int get (const fmr::Phys_info, const fmr::Local_int &){return 1;}
#endif
  protected:
    int prep () final override;
    int chck () final override;
    int exit_task (int  err) final override;
  private:
//    int init_task (int* argc, char** argv)final override;//TODO called?
    int run  () final override;
    //fmr::Val<fmr::Local_int> part_n ={0, fmr::math::Real, fmr::Data::Testing};
    //fmr::Val<fmr::Local_int> part_n = fmr::Val<fmr::Local_int>();//TODO need?
    //TODO fmr::Val<fmr::Local_int>(fmr::Int) or Nat?
  public:
    virtual ~Frun ()   noexcept=default;// Virtual destructor for virtual class
    Frun (Sims*)       noexcept;
    Frun ()                    =delete;
    Frun (Frun const&)         =delete;// not copyable
    Frun operator=(const Frun&)=delete;
};
}//end Femera namespace
//end FMR_HAS_FRUN_HPP
#endif
