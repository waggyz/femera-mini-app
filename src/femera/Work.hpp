#ifndef FEMERA_HAS_WORK_HPP
#define FEMERA_HAS_WORK_HPP
/** */
#include "../fmr/perf/Meter.hpp"
#include "../fmr/math.hpp"
#include "Errs.hpp"
#include "type.hpp"

#include <memory>     // std::shared_ptr, std::make_shared
#include <vector>
#include <deque>
#include <tuple>
#if 0
#include <utility>    // std::forward
#define FMR_SMART_PTR std::unique_ptr
//#define FMR_MAKE_ME_SMART(T) std::unique_ptr<T>(new T (std::forward<Args>(args)...))
#define FMR_MAKE_SMART(T) fmr::make_unique<T> // in Work.ipp (from C++14)
#else
#define FMR_SMART_PTR std::shared_ptr
#define FMR_MAKE_SMART(T) std::make_shared<T>
#endif
/* Forward declares -----------------------------------------------------------
 * This describes the internaland external namespace and class hierarchy for
 * all modules, including those not implemented or available yet.
 */
namespace femera {
  class Work;                          // abstract (pure virtual) base class
  template <typename> class Proc;      // abstract CRTP base derived from Work
  namespace proc {                     // CRTP inheriance from Proc
    class Main;                        // public interface for process handling
    class Root; class Node; class Fcpu;// Femera process handling
    class Fmpi; class Fomp; class Nvid;// process handling libraries
} }//end femera::proc:: namespace
namespace femera {
  template <typename> class Data;      // abstract CRTP base derived from Work
  namespace data {                     // CRTP inheriance from Data
    class File;                        // public interface for data handling
    class Logs; class Dlim; class Text;// Femera data handling
    class Bank;// class View;          // View here or as a CRTP base? 
    class Cgns; class Gmsh; class Pets;// data handling libraries
    class Moab;
} }//end femera::data:: namespace
namespace femera {
  template <typename> class Test;      // abstract CRTP base derived from Work
  namespace test {                     // CRTP inheriance from Test
    class Beds;                        // public interface for test handling
    class Unit; class Self; class Perf;// Femera testing
    class Gtst;                        // testing libraries
} }//end femera::test:: namespace
namespace femera {
  template <typename> class Task;      // abstract CRTP base derived from Work
  namespace task {                     // CRTP inheriance from Task
    class Jobs;                        // public interface for task handling
    class Sims; class Runs;            // Femera tasks
    //class Post;
    //class Plug;                      // plugins
} }//end femera::task:: namespace
// typedefs -------------------------------------------------------------------
namespace femera {
  using Work_spt = FMR_SMART_PTR <Work>;      // abstract packaging base class
  using Jobs_spt = FMR_SMART_PTR <task::Jobs>;// concrete Task interface
}//end femera:: namespace
namespace femera {
  // Base class ===============================================================
  class Work {/* This is an abstract (pure virtual) base class (interface).
  * Derived classes use the curiously recurrent template pattern (CRTP) e.g.,
  * class Proc : public Work { .. };
  * class Main : public Proc <Main> { friend class Proc; .. };
  */
  public:// typedefs
    using Core_ptrs_t = std::tuple <proc::Main*, data::File*, test::Beds*>;
    using Task_path_t = std::vector <fmr::Local_int>;
    using Work_time_t = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>;
  private:
    using Task_stck_t = std::deque <Work_spt>;
  public:// variables ---------------------------------------------------------
    // classes with (mostly) only member methods public
    Work_time_t time = Work_time_t (); // performance timer, NOT thread-safe
    proc::Main* proc = nullptr;        // processing hierarchy
    data::File* data = nullptr;        // data, logging, and file handling
    test::Beds* test = nullptr;        // correctness and performance testing
  protected:// variables
    Task_stck_t task_list ={};
    std::string      name ="unknown work";
    std::string      abrv ="work";
    std::string   version ="";
    Work_type   task_type = task_cast (Base_type::Work);
#if 0
/*
https://stackoverflow.com/questions/60040665
/replacing-the-command-line-arguments-int-argc-and-char-argv-with-stdvectors
*/
    std::string argv_prfx ="fmr:";
    std::unique_ptr<char*[]> my_argv;//use: err= init (&my_argc, my_argv.get());
    int                      my_argc =0;
#endif
  private:// variables
    bool did_work_init = false;
    bool  is_work_main = true ;// save for use after proc::exit (..)
  protected:// variables visible to CRTP children with friend class
    bool        set_init     (bool) noexcept;// sets & returns did_work_init
  public:// methods -----------------------------------------------------------
    template <typename T, typename C> static constexpr
    T* cast_via_work (C* child) noexcept;
    Core_ptrs_t get_core     () noexcept;
    std::string get_abrv     () noexcept;
    std::string get_version  () noexcept;
    std::string get_name     () noexcept;
    std::string set_name     (const std::string&) noexcept;
    bool        did_init     () noexcept;//     returns did_work_init
    // task stack handling
    fmr::Local_int get_task_n () noexcept;
    fmr::Local_int add_task   (Work_spt) noexcept;// returns task number added
    fmr::Local_int del_task   (fmr::Local_int ix) noexcept;// returns task_n
    //NOTE more efficient to get_task (..) then check for nullptr
    //     instead of if (has_task (..)) {get_task (..);}
    bool has_task (const Task_path_t&)  noexcept;//TODO needs tests
    bool has_task (Work_type, fmr::Local_int ix=0) noexcept;//TODO needs tests
  public:// pure virtual methods defined in CRTP base classes derived from Work
    virtual std::string get_base_abrv () noexcept =0;
    // intialize and exit
    virtual fmr::Exit_int init (int* argc, char** argv) noexcept =0;
    virtual fmr::Exit_int exit (fmr::Exit_int err=0)    noexcept =0;
  protected:// methods
    std::string set_abrv (const std::string&) noexcept;
    // Work task_list initialization and exit
    fmr::Exit_int init_list (int* argc, char** argv) noexcept;// init forward
    fmr::Exit_int exit_list () noexcept;// exit task_list in reverse
    fmr::Exit_int exit_tree () noexcept;// exit task_list hierarchy in reverse
    // below called by Derived::get_task_*(..)
    Work* get_work (fmr::Local_int) noexcept;
    Work* get_work (Work_type, fmr::Local_int ix=0) noexcept;
    Work* get_work (const Task_path_t&) noexcept;
  private:// methods
    fmr::Local_int  log_init_list () noexcept;// returns # of tasks initialized
    std::string get_did_init_list () noexcept;// returns space-seperated list
    std::string get_not_init_list () noexcept;// returns space-seperated list
    fmr::Exit_int exit_info (Work*, fmr::perf::Float busy_s);
  protected:// Built-in methods called only by child classes ------------------
    Work (Work::Core_ptrs_t) noexcept;// preferred constructor
    Work () noexcept;// default constructor for early parts of initialization
    Work (Work&&);              // is movable
    Work& operator
      =  (const Work&) =delete; // not asignable
    Work (const Work&) =delete; // not copyable
    virtual ~Work ();// not inline to avoid -Winline large growth warning
  };// ========================================================================
}  //end femera:: namespace
#include "Work.ipp"
 //
//end FEMERA_HAS_WORK_HPP
#endif
