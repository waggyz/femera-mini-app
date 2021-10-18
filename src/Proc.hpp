#ifndef FMR_HAS_PROC_HPP
#define FMR_HAS_PROC_HPP
/** */
#include "Work.hpp"

#include <valarray>
#include <set>

#include <thread>     // hardware_concurrency (ncpu)

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
class Sims;
class Flog;
class Proc : public Work {
  // typedefs -----------------------------------------------------------------
  public:
    struct Locality {
      int hier_lv =-1;
      int thrd_n  = 0;
      int thrd_id = 0;
      int numa_id = 0;
      int phys_id = 0;
      int logi_id = 0;
      int is_mast = int(true);// same as other types for easier sync
    };
    typedef uint64_t Team_id;// cast-compatible with ::MPI_comm from mpi.h
  // variables ----------------------------------------------------------------
  protected://TODO Make private with get/setters.
    int my_master = 0;
    int hier_lv   =-1;// default <0 : not in processing hierarchy
    int proc_n    = 1;
    int core_n    = 1;
    int logi_n    = 1;
    Team_id team_id = 0;
  private:
    std::set<int> handled_opts ={};
  public:
    Flog* log  = nullptr;//TODO make new Flog objects for each derived instance.
    std::vector<Proc*> hier = {};
  // methods ------------------------------------------------------------------
  private:
    bool is_opt_handled (int);
  protected:
    int init_task (int* argc, char** argv) override;
    int exit_task (int  err) override;
    int prep () override;//NOTE Derived should final override this.
  public:
    virtual ~Proc ();// not default: deletes log
    Proc ();// not default
    Proc (const Proc&)=delete;
    Proc operator= (const Proc&)=delete;
    //
    std::string print_summary () final override;
    std::string print_details () final override;
    int chck () override;
    //
    int opt_add (const int);
    //
    int lvl_thrd_n  (const int lvl);// total threads below this processing level
    int get_numa_n  ();
    int get_numa_id ();
    int get_core_id ();
    int get_logi_id ();
    int get_hier_n  ();
    int get_hier_lv ();
    //
    std::valarray<Locality> get_stat ();
    std::valarray<Locality> all_stat ();// Parallel environment snapshot
    //
    virtual bool is_in_parallel ();
    virtual bool is_master   ();
    virtual int  get_proc_id ();
    virtual int  get_proc_n  ();//NOTE Use for information only.
    virtual int  set_proc_n  (const int);// try to set, then return get_proc_n()
    //TODO combine: run (Sims*, fmr::Schedule, fmr::Concurrency)
#if 0
    virtual int run_sync (Sims*);
    virtual int run_list (Sims*);
    virtual int run_fifo (Sims*);
#else
    virtual int run (Sims*);
#endif
//    virtual int run_batch (Sims*);
//    virtual int run (Sims*, fmr::Schedule, fmr::Concurrency);
    //
    inline virtual Team_id get_team_id ();// can cast MPI_Comm
    inline virtual int barrier ();
    inline virtual std::valarray<int> gather (std::valarray<int>);
    inline virtual std::string        reduce (std::string);
};
// inline functions ===========================================================
inline Proc::Team_id Proc::get_team_id (){
  return this->team_id;
}
int Proc::barrier (){int err=0;
#if 0
  if(this->work_type == work_cast(Base_type :: Proc) ){
    const int n = Proc::task.count();
    for(int i=1; i<n; i++){
      // Start at 1 because I am the first thing on my todo_list.
      err+=abs( Proc::task.get<Proc>(i)-> barrier () );
  } }
#else
  if (this->work_type == work_cast (Base_type::Proc)){
    const int n = Proc::task.count ();
    for (int i=0; i<n; i++){
      auto P=Proc::task.get<Proc>(i);
      if (P != this){// Avoid infinite recursion.
        err += abs (P->barrier ());
  } } }
#endif
  return err==0 ? 0 : 1;
}
inline std::string Proc::reduce (std::string strings){
#ifdef FMR_DEBUG
  printf("Proc::reduce(strings)\n");
#endif
  if (this->work_type == work_cast (Base_type::Proc)){
    // Reduce the task->todo hierarchy.
#if 0
    for(int i=1; i<this->task.count(); i++){
      // Start at 1 because I am the first thing on my todo list.
      strings = this->task.get<Proc>(i)->reduce( strings );
  } }
#else
    const int n = Proc::task.count ();
    for (int i=0; i<n; i++){
      auto P=Proc::task.get<Proc>(i);
      if (P != this){// Avoid infinite recursion.
        strings = P->reduce (strings);
  } } }
#endif
  return strings;
}
inline std::valarray<int> Proc::gather (std::valarray<int> vals){
#if 0
  if(this->work_type == work_cast(Base_type::Proc)){
    // Gather the task->todos.
    for(int i=1; i<Proc::task.count(); i++){
      // Start at 1 because I am the first thing on my todo_list.
      vals = Proc::task.get<Proc>(i)->gather (vals);
#ifdef FMR_DEBUG
      Proc::get_task<Proc>(i)->log.printf("gather todo[%i/%i]: %s size: %i\n", i,
        int(Proc::task.count()), Proc::get_task<Proc>(i)->version.c_str(),
        int(Proc::task.get<Proc>(i)->count()) );
#endif
  } }
#else
  if (this->work_type == work_cast (Base_type::Proc)){
    // Gather the task->todos.
    const int n = Proc::task.count ();
    for (int i=0; i<n; i++){
      auto P=Proc::task.get<Proc>(i);
      if (P != this){// Avoid infinite recursion.
        vals = P->gather (vals);
  } } }
#endif
  return vals;
}
}//end Femera namespace

#if 0
namespace fmr {namespace loop {//TODO Move to another file?
  static inline bool round_robin (const int thrd_n, const int thrd_i,
    const int item_n, int* item_i){
    item_i[0] += (item_i[0]==0) ? thrd_i : thrd_n;
    return item_i[0] < item_n;
  };
  //
  // XXX_FMR_PRAGMA_OMP(omp parallel firstprivate(fname_i))
  // while (fmr::loop::round_robin (this->proc, 0, &fname_i) < fname_n){
  //
  static inline bool round_robin (Femera::Proc* P,
    const int proc_l, const int item_n, int* item_i){
    std::valarray<Locality> pstat;
    if (item_i==0){ pstat = P->get_stat (); }
    item_i += (item_i==0) ? pstat[proc_l].thrd_i : pstat[proc_l].thrd_n;
    return item_i < item_n;
  };
  /* This might work.
    int fname_i=0;
    XXX_FMR_PRAGMA_OMP(omp parallel firstprivate(fname_i))
    while (fmr::loop::round_robin (this->proc, 0, fname_n, &fname_i)){
      // Do some on fname_i (in parallel).
    }
  */
} }//end fmr::loop namespace
#endif

//end FMR_HAS_PROC_HPP
#endif
