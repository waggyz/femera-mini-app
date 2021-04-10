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
  typedef std::vector<fmr::Data> Data_list;
  // member variables --------------------------------------------------------
  // this->task contains Frun instances to run sims distributed by this Sims.
  protected:
    Data_list data_list = {};
  public://TODO protected:
    // These data storage types are available to all derived classes.
    std::map<fmr::Data,fmr::Dim_int_vals>       dims ={};
    std::map<fmr::Data,fmr::Enum_int_vals>     enums ={};
    std::map<fmr::Data,fmr::Local_int_vals>   locals ={};
    std::map<fmr::Data,fmr::Global_int_vals> globals ={};
    /*
     * Each Sims     data.size = sims_bats_sz * N : N=1 scalars, or array size
     *
     * Frun geometry data.size = part_bats_sz * N : partitioned geometry
     *               data.size = mesh_bats_sz * N : unpartitioned (merged) geom.
     *
     * Part geometry data.size = part_bats_sz * N : sub-partitioned geometry
     *               data.size = mesh_bats_sz * N : terminal partition
     *
     * Mesh          data.size = elem_bats_sz * N : element data
     *               data.size = node_bats_sz * N : node data
     */
    Sims* parent = nullptr;
    std::string model_name ="";// Set in prep(), if not before.
    std::deque<std::string> model_list ={};
    //
    fmr::Distribute from//TODO Change "from" to "here" ?
      = {1, fmr::Schedule::List, fmr::Concurrency::Independent};
    fmr::Distribute send
      = {2, fmr::Schedule::Fifo, fmr::Concurrency::Independent};
    //
    fmr::Local_int sims_ix = 0;// collection number
    fmr::Sim_size sims_size = fmr::Sim_size::XS;//TODO fmr::Sim_size::Unknown
  protected:
    fmr::Partition part_type = fmr::Partition::None;
    fmr::Dim_int tree_lv = 0;// sim/sim collection depth
  private:
    int get_data_vals (const std::string model_name, const Data_list list);
  public:
    fmr::Dim_int geom_d =0;// used by inheriting classes
  // methods -----------------------------------------------------------------
  public:
    virtual int add   (const std::string model_name);
    virtual int run   ();
    virtual int clear ();
    //
    fmr::Dim_int get_dim_val (const fmr::Data, const size_t ix);
    fmr::Enum_int get_enum_val (const fmr::Data, const size_t ix);
    fmr::Local_int get_local_val (const fmr::Data, const size_t ix);
    fmr::Global_int get_global_val (const fmr::Data, const size_t ix);
    //
    fmr::Local_int get_sims_n ();
    fmr::Local_int get_frun_n ();
//    fmr::Local_int get_mtrl_n ();//TODO
    fmr::Local_int get_part_n ();// used in derived classes
    int prep () override;//TODO or virtual int init() ?
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
