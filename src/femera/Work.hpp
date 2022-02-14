#ifndef FEMERA_HAS_WORK_HPP
#define FEMERA_HAS_WORK_HPP
/** */
#include "../fmr/perf/Meter.hpp"
#include "Errs.hpp"
//#include "Task.hpp"
//#include "vals.hpp"

#include <memory>     //TODO std::shared_ptr, std::make_shared
#include <string>
#include <vector>
#include <deque>
#include <tuple>
#if 0
#define FMR_SMART_PTR std::unique_ptr
#define FMR_MAKE_SMART(T) std::unique_ptr<T>(new T (std::forward<Args>(args)...))
//define FMR_MAKE_SMART std::make_unique // in Work.ipp (from C++14)
#else
#define FMR_SMART_PTR std::shared_ptr
#define FMR_MAKE_SMART std::make_shared
#endif

namespace femera {
  // Forward declares
  class Work;
  template <typename> class Proc;// class Main; interface
  template <typename> class Data;// class File; interface
  template <typename> class Test;// class Beds; interface
  template <typename> class Sims;// class Jobs; interface
  namespace proc {
    class Main; class Root; class Node; class Fmpi; class Fomp; class Fcpu;
    class Nvid; }
  namespace data { class File; class Logs; }// class Type; class Base; }//TODO Fake -> Base?
  namespace test { class Beds; class Unit; class Self; class Perf; class Gtst; }
  namespace sims { class Jobs; }
  // typedefs
  using Work_spt = FMR_SMART_PTR <Work>;
#if 0
  using Main_t = FMR_SMART_PTR <proc::Main>;// concrete Proc interface
  using Root_t = FMR_SMART_PTR <proc::Root>;
  using Node_t = FMR_SMART_PTR <proc::Node>;
  using Fmpi_t = FMR_SMART_PTR <proc::Fmpi>;
  using Fomp_t = FMR_SMART_PTR <proc::Fomp>;
  using Fcpu_t = FMR_SMART_PTR <proc::Fcpu>;
  //
  using File_t = FMR_SMART_PTR <data::File>;// concrete Data interface
  using Logs_t = FMR_SMART_PTR <data::Logs>;
  //
  using Beds_t = FMR_SMART_PTR <test::Beds>;// concrete Test interface
  //using Unit_t = FMR_SMART_PTR <test::Unit>;// built-in unit tests
  //using Self_t = FMR_SMART_PTR <test::Self>;// built-in integration tests
  //using Perf_t = FMR_SMART_PTR <test::Perf>;// built-in performance tests
  using Gtst_t = FMR_SMART_PTR <test::Gtst>;
#endif
  using Jobs_spt = FMR_SMART_PTR <sims::Jobs>;// concrete Sims interface
  //
  template <typename T, typename U>
  T* cast_via_work (U* obj);
  //
  class Work {/* This is an abstract (pure virtual) base class (interface).
  * Derived classes use the curiously recurrent template pattern (CRTP) e.g.,
  * class Proc : public Work { .. };
  * class Main : public Proc<Main> { private: friend class Proc; .. };
  */
  public:// typedefs ----------------------------------------------------------
    using Core_ptrs   = std::tuple  <proc::Main*, data::File*, test::Beds*>;
    using Task_list_t = std::deque  <Work_spt>;
    using Task_path_t = std::vector <fmr::Local_int>;
    using Task_tree_t = std::vector <Task_path_t>;
  public:// variables ---------------------------------------------------------
    fmr::perf::Meter time = fmr::perf::Meter ();
    //
    proc::Main* proc = nullptr;// processing hierarchy
    data::File* data = nullptr;// data and file handling
    test::Beds* test = nullptr;// correctness and performance testing
    //
    std::string name ="unknown work";
    std::string abrv ="work";
  protected:
    std::string   version ="";
    Task_list_t task_list ={};
#if 0
// https://stackoverflow.com/questions/60040665/replacing-the-command-line-arguments-int-argc-and-char-argv-with-stdvectors
    std::string argv_prfx ="-fmr:";
    std::unique_ptr<char*[]> my_argv;// use: err= init (&my_argc, my_argv.get());
    int                      my_argc =0;
#endif
  protected:
    fmr::Dim_int info_d    = 1;
  private:
    bool did_work_init = false;
    bool  is_work_main = true ;// save for use after proc::exit (..)
  public:// methods -----------------------------------------------------------
    //NOTE Make at least 1 method pure virtual.
    //FIXME Do all virtual methods need to be pure
    //      to avoid vtable using CRTP derived classes?
    virtual fmr::Exit_int init (int* argc, char** argv) noexcept =0;
    virtual fmr::Exit_int exit (fmr::Exit_int err=0)    noexcept =0;
    virtual std::string get_base_name () =0;
    //
//TODO Task_tree_t get_tree     ()         noexcept;
    fmr::Local_int get_task_n   ()         noexcept;
    fmr::Local_int add_task     (Work_spt) noexcept;// returns task number added
    fmr::Local_int del_task     (fmr::Local_int ix) noexcept;// returns task_n
/*
 *  Derived_t      new_task     ()// in Derived and returns that smart pointer
 *  Derived_t      new_task     (Core_ptrs)     //           ""
 *  Derived_t      get_task_spt (fmr::Local_int)//           ""
 *  Derived*       get_task_raw (fmr::Local_int)// returns Derived* raw pointer
 */
    Core_ptrs get_core () noexcept;
  public:
    Work*    get_work_raw (fmr::Local_int) noexcept;
    Work*    get_work_raw (Task_path_t)    noexcept;
    Work_spt get_work_spt (fmr::Local_int) noexcept;
    Work_spt get_work_spt (Task_path_t)    noexcept;
    // above called by Derived::get_task_*(..)
    //
  protected:
    // Work stack initialization and exit
    fmr::Exit_int init_list (int* argc, char** argv) noexcept;// init forward
    fmr::Exit_int exit_list () noexcept;// exit task_list in reverse
    fmr::Exit_int exit_tree () noexcept;// exit task hierarchy in reverse
  protected:// Built-in stuff -------------------------------------------------
    Work ()            =default;
    Work (Work const&) =default;// copyable
    Work& operator=
      (const Work&)    =default;
  public:
    virtual ~Work ();// not inline to avoid -Winline large growth warning
  };
}//end femera:: namespace

#include "Work.ipp"

#undef FMR_DEBUG
//end FEMERA_HAS_WORK_HPP
#endif

#if 0
  // to avoid FMR_CAST_TO_DERIVED use in task->get* methods?
  // Variables ----------------------------------------------------------------
  public:
    Work_type    work_type = work_cast (Base_type::Work);
//    Work_type    base_type = work_cast (Base_type::Work);//TODO Remove this?
    std::string  task_name ="Femera work";
    std::string    version ="";
    Proc*             proc = nullptr;
    Data*             data = nullptr;
    Task<Work>        task = Task<Work>(this);// Work task stack
    // task must be public because parent accesses derived instances.
    fmr::perf::Meter  time = fmr::perf::Meter ();
    std::string meter_unit ="unit";
    int verblevel = 1;
  protected:
  private:
  // Methods ------------------------------------------------------------------
  public:
    Work           ()           =default;
    Work           (Work const&)=delete;// not copyable
    void operator= (const Work&)=delete;
    virtual ~Work  ()=0;// pure virtual destructor indicates Work is abstract
    //
    int print_task_time (const std::string label_suffix);
    //
    // Work stack initialization and exit
    int init (int* argc, char** argv);//TODO init_work or init_all or init_stack?
    int exit (int  err);              //TODO init_work or exit_all or exit_stack?
    //
    // Single-inheritance interface
    // These pure virtual functions must be specialized for each task.
    virtual int init_task (int* argc, char** argv)=0;//TODO Change to init(..).
    virtual int exit_task (int  err)=0;//TODO               Change to exit(..).
    //
    // If a derived class does not override these, then,
    // e.g., a call to Derived::prep() will call Work::prep().
    virtual int prep ();//TODO or virtual int init() ?
    virtual int chck ();//TODO chk?
    //
    virtual std::string print_summary ();
    virtual std::string print_details ();
  protected:
  private:
#endif
