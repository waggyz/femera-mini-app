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
   * into its task stack.
   * */
  // typedefs ---------------------------------------------------------------
  // member variables -------------------------------------------------------
  public:
  protected:
#if 0
    using Globals = fmr::Global_int_vals;
    using Locals = fmr::Local_int_vals;
    using Enums = fmr::Enum_int_vals;
    using Dims = fmr::Dim_int_vals;
    /* The following have values for each sim in this collection,
     * bundled for this->data->read_sims_data (&this->dims,&this->enums,...).
     *    i.e. *.data.size() == this->get_sims_n().
     * But, if *.data.size() == 0, assume 0 (or *::None) for all sims
     * and, if *.data.size() == 1, apply the same value to all sims.
     */
    //TODO protected/public: Init in constructors, or after sims_n is known.
    std::map<fmr::Data,Globals>globals = {};
    std::map<fmr::Data,Locals> locals = {
      {fmr::Data::Sims_n,      Locals (fmr::Data::Sims_n)},// child sub-sims
      {fmr::Data::Gset_n,      Locals (fmr::Data::Gset_n)},
      {fmr::Data::Part_n,      Locals (fmr::Data::Part_n)},
      {fmr::Data::Part_halo_n, Locals (fmr::Data::Part_halo_n)},// shared surfs
      {fmr::Data::Mtrl_n,      Locals (fmr::Data::Mtrl_n)}
    };
    std::map<fmr::Data,Enums>  enums = {
      {fmr::Data::Sims_type,   Enums  (fmr::Data::Sims_type)},
      {fmr::Data::Time_type,   Enums  (fmr::Data::Time_type)}
    };
    std::map<fmr::Data,Dims>   dims = {//TODO std::queue, std::unordered_map ?
      {fmr::Data::Geom_d,      Dims   (fmr::Data::Geom_d)},
      {fmr::Data::Phys_d,      Dims   (fmr::Data::Phys_d)}
    };
#endif
  private:
    fmr::Local_int sims_ix = 0;// sim number
    fmr::Dim_int   sims_lv = 1;// subsim hierarchy level
  // methods ----------------------------------------------------------------
  protected:
    int prep () final override;
    int chck () final override;
    int exit_task (int  err) final override;
  private:
//    int init_task (int* argc, char** argv)final override;//TODO called?
    int run  () final override;
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
