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
#if 1 //TODO Remove.
  protected:
    fmr::Data_id data_id = "(unknown)";
  public:
    virtual fmr::Data_id make_id ();
    virtual fmr::Data_id get_id ();
//    fmr::Local_int get_sims_ix (std::string sim_name);
#endif
#if 1
  protected:
    fmr::Local_int_vals part_dims// geom_d, part_n, mesh_n,...
      = fmr::Local_int_vals (fmr::Data::Geom_info, 0);
    fmr::Local_int_vals phys_dims// phys_d, mtrl_n
      = fmr::Local_int_vals (fmr::Data::Phys_info, 0);
    //TODO Remove and replace above with below.
#endif
#if 1
  protected:
    // These data storage types are available to all derived classes.
    std::map<fmr::Data,fmr::Global_int_vals> globals ={};
    std::map<fmr::Data,fmr::Local_int_vals>   locals ={};
    std::map<fmr::Data,fmr::Enum_int_vals>     enums ={};
    std::map<fmr::Data,fmr::Dim_int_vals>       dims ={};
#endif
#if 0
    using Globals = fmr::Global_int_vals;
    using Locals  = fmr::Local_int_vals;
    using Enums   = fmr::Enum_int_vals;
    using Dims    = fmr::Dim_int_vals;
    /* The following have values for each sim in this collection,
     * bundled for this->data->read_sims_data (&this->dims,&this->enums,...).
     * i.e.    *.data.size() == this->get_sims_n().
     * But, if *.data.size() == 0, assume 0 (or *::None) for all sims
     * and, if *.data.size() == 1, apply the same value to all sims.
     */
    //TODO Initialize in constructors, or after sims_n is known.
    std::map<fmr::Data,Globals> globals ={};
    std::map<fmr::Data,Locals>  locals ={
      {fmr::Data::Sims_n,       Locals (fmr::Data::Sims_n)},// child sims for
      //                                                       each sim
      {fmr::Data::Gset_n,       Locals (fmr::Data::Gset_n)},
      {fmr::Data::Part_n,       Locals (fmr::Data::Part_n)},
      {fmr::Data::Part_halo_n,  Locals (fmr::Data::Part_halo_n)},//shared surfs
      {fmr::Data::Mtrl_n,       Locals (fmr::Data::Mtrl_n)}
    };
    std::map<fmr::Data,Enums>   enums ={
      {fmr::Data::Sims_type,    Enums (fmr::Data::Sims_type)},
      {fmr::Data::Time_type,    Enums (fmr::Data::Time_type)}
    };
    std::map<fmr::Data,Dims>    dims ={
      {fmr::Data::Geom_d,       Dims (fmr::Data::Geom_d)},
      {fmr::Data::Phys_d,       Dims (fmr::Data::Phys_d)}
    };
#endif
  public://TODO protected:
    Sims* parent = nullptr;
    std::string model_name ="(unnamed model collection)";
    std::deque<std::string> model_list ={};
    //
    fmr::Distribute from = {1, fmr::Schedule::Once, fmr::Concurrency::Once};
    fmr::Distribute send = {2, fmr::Schedule::Fifo,
      fmr::Concurrency::Independent};
  protected:
    fmr::Partition  part_algo = fmr::Partition::None;
    //
    fmr::Local_int sims_ix = 0;// collection number
    fmr::Dim_int   sims_lv = 0;// independent sim collection depth
  // methods -----------------------------------------------------------------
  public:
    virtual int add   (const std::string model_name);
    virtual int run   ();//TODO or start () ?
    virtual int clear ();
#if 0
    fmr::Local_int get_part_n (std::string sim_name, fmr::Tree_path part);
    fmr::Dim_int   get_geom_d (std::string sim_name, fmr::Tree_path part);
    fmr::Dim_int   get_phys_d (std::string sim_name, fmr::Tree_path part);
    fmr::Local_int get_mtrl_n (std::string sim_name, fmr::Tree_path part);
#endif
    //
    fmr::Local_int get_sims_n ();
//    fmr::Local_int get_mtrl_n ();//TODO
    fmr::Local_int get_part_n ();// used in derived classes
  protected:
    //TODO The following are run by this->run () ?
    int chck () override;//TODO Is this needed here?
#if 0
    virtual int iter ();//TODO or loop() ?
    virtual int krnl ();//TODO or body() ? Is this needed?
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
    Sims           (Proc*, Data*) noexcept;
    Sims           (Sims const&) =delete;// not copyable
    Sims operator= (const Sims&) =delete;
  protected:
    Sims ()                      =default;// called implicitly by children
};
}//end Femera namespace

#if 0
    virtual int init (){int err=0;
      for (int task_i = this->dist->get_init_i ();
               task_i < this->get_task_n () && !this->dist->is_done ();
               task_i = this->dist->get_task_i ()){
        err+= this->get_task<Sims>(task_i)->init ();
      }
      return err;
    }
    virtual int iter (){int err=0;
      for (int task_i = this->dist->get_init_i ();
               task_i < this->get_task_n () && !this->dist->is_done ();
               task_i = this->dist->get_task_i ()){
        err+= this->get_task<Sims>(task_i)->iter ();
      }
      return err;
    }
      virtual int run () {int err=0;
      FMR_PRAGMA_OMP(omp parallel) {int perr=0;
        perr+= this->prep ();
        perr+= this->iter ();
        perr+= this->sync->run ();
        perr+= this->post->run ();
        perr+= this->done (err);
        //TODO reduce sum perr to err?
      }
      return err;
    }
#endif

#if 0
sims.init();
int done=0; while( !done ){
  done = sims.iter( iter_batch_n, iter_max, rtol, atol );
  this->proc.printf_labeled("Iteration"+std::to_string(sims.iter_i),
    "blah blah..." );
}
sims.exit( sims.err );
#endif

#undef FMR_DEBUG
//end FMR_HAS_FSIM_HPP
#endif
