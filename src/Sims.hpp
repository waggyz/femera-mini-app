#ifndef FMR_HAS_FSIM_HPP
#define FMR_HAS_FSIM_HPP
/** */

#include <vector>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif
namespace Femera {
class Sims : public Work {// simulation collection manager
  /* This is the simulation collection manager. There is one of these for each
   * collection, residing in the main task stack, and running at the default
   * (usually MPI world) processing level. It sets up its stack for serial runs
   * of sims at the same level, or for running sims in parallel at a deeper
   * (e.g. MPI team, OpenMP, or GPU) level.
   */
  // typedefs ----------------------------------------------------------------
  // member variables --------------------------------------------------------
  // this->task contains Frun instances to run sims distributed by this Sims.
  public:
    // These data storage types are available to all derived classes.
    std::map<fmr::Data,fmr::Dim_int_vals>       dims ={};
    std::map<fmr::Data,fmr::Enum_int_vals>     enums ={};
    std::map<fmr::Data,fmr::Local_int_vals>   locals ={};
    std::map<fmr::Data,fmr::Global_int_vals> globals ={};
  public://TODO protected:
    Sims* parent = nullptr;
    std::string model_name ="";// Set in prep(), if not before.
    std::deque<std::string> model_list ={};
    //
    fmr::Distribute from
      ={1,fmr::Schedule::List,fmr::Concurrency::Independent};
    //TODO from => here ?
    fmr::Distribute send ={2,fmr::Schedule::Fifo,fmr::Concurrency::Independent};
    //
    fmr::Sim_size sims_size = fmr::Sim_size::XS;//TODO fmr::Sim_size::Unknown
  protected:
    fmr::Partition  part_type = fmr::Partition::None;
    //
    fmr::Local_int sims_ix = 0;// collection number
    fmr::Dim_int   tree_lv = 0;// sim/sim collection depth
  // methods -----------------------------------------------------------------
  public:
    virtual int add   (const std::string model_name);
    virtual int run   ();
    virtual int clear ();
    //
    fmr::Local_int get_sims_n ();
    fmr::Local_int get_frun_n ();
//    fmr::Local_int get_mtrl_n ();//TODO
    fmr::Local_int get_part_n ();// used in derived classes
  protected:
    //TODO The following are run by this->run () ?
    int chck () override;//TODO Is this needed here?
#if 0
    virtual int iter ();//TODO or loop() ?
    virtual int krnl ();//TODO or body() ? Is this needed?
    virtual int sync ();
    virtual int done ();//TODO or exit() ? Is this needed here?
    virtual int post ();// post->run (); ?
    virtual int exit ();//TODO or done() ? Needed, or just use Work::exit(err)?
#endif
    int exit_task (int err) override;
  private:
    int init_task (int* argc, char** argv) final override;//TODO called?
    int prep () override;//TODO or virtual int init() ?
  // constructors and destructors --------------------------------------------
  public:
    virtual ~Sims  ()   noexcept =default;
    Sims           (Proc*,Data*)  noexcept;
    Sims           (Sims const&) =delete;// not copyable
    Sims operator= (const Sims&) =delete;
  protected:
    Sims ()                      =default;// called implicitly by children
};
}//end Femera namespace

#undef FMR_DEBUG
//end FMR_HAS_FSIM_HPP
#endif
