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

namespace femera {
  // Forward declares ---------------------------------------------------------
  class Work;                          // abstract (pure virtual) base class
  template <typename> class Proc;      // abstract CRTP base derived from Work
  namespace proc {                     // CRTP inheriance from Proc
    class Main;                        // public interface to process handling
    class Root; class Node; class Fcpu;// Femera process handling
    class Fmpi; class Fomp; class Nvid;// process handling libraries
  }
  template <typename> class Data;      // abstract CRTP base derived from Work
  namespace data {                     // CRTP inheriance from Data
    class File;                        // public interface to data
    class Logs; class Dlim; class Text;// Femera data handling
    class Bank;// class View;
    class Cgns; class Gmsh; class Moab;// data handling libraries
    class Pets;
  }
  template <typename> class Test;      // abstract CRTP base derived from Work
  namespace test {                     // CRTP inheriance from Test
    class Beds;                        // public interface to tests
    class Unit; class Self; class Perf;// Femera testing
    class Gtst;                        // testing libraries
  }
  template <typename> class Task;      // abstract CRTP base derived from Work
  namespace task {                     // CRTP inheriance from Task
    class Jobs;                        // public interface to tasks
    class Sims; class Runs;            // Femera tasks
    //class Post;
    //class Plug;                      // plugins
  }
  // typedefs -----------------------------------------------------------------
  using Work_spt = FMR_SMART_PTR <Work>;      // abstract packaging base class
  using Jobs_spt = FMR_SMART_PTR <task::Jobs>;// concrete Task interface
  // Base class ===============================================================
  class Work {/* This is an abstract (pure virtual) base class (interface).
  * Derived classes use the curiously recurrent template pattern (CRTP) e.g.,
  * class Proc : public Work { .. };
  * class Main : public Proc <Main> { private: friend class Proc; .. };
  */
  public:// typedefs
    using Core_ptrs_t = std::tuple <proc::Main*, data::File*, test::Beds*>;
    using Task_path_t = std::vector <fmr::Local_int>;
    using Work_time_t = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>;
  public:// variables ---------------------------------------------------------
    // classes with (mostly) only member methods public
    Work_time_t time = Work_time_t ();// performance timer, NOT thread-safe
    proc::Main* proc = nullptr;       // processing hierarchy
    data::File* data = nullptr;       // data, logging, and file handling
    test::Beds* test = nullptr;       // correctness and performance testing
  private:// variables
    using Task_stck_t = std::deque <Work_spt>;
  protected:// variables
    std::string       name ="unknown work";
    std::string       abrv ="work";
    std::string    version ="";
    Task_stck_t  task_list ={};
    Work_type    task_type = task_cast (Base_type::Work);
    fmr::Dim_int info_d    = 1;
#if 0
// https://stackoverflow.com/questions/60040665
    /replacing-the-command-line-arguments-int-argc-and-char-argv-with-stdvectors
    //
    std::string argv_prfx ="-fmr:";
    std::unique_ptr<char*[]> my_argv;//use: err= init (&my_argc, my_argv.get());
    int                      my_argc =0;
#endif
  private:// variables
    bool did_work_init = false;
    bool  is_work_main = true ;// save for use after proc::exit (..)
  public:// methods -----------------------------------------------------------
    template <typename T, typename C> static constexpr
    T* cast_via_work (C* child) noexcept;
    std::string set_name    (const std::string&) noexcept;
    std::string get_name    () noexcept;
    std::string get_abrv    () noexcept;
    std::string get_version () noexcept;
    Core_ptrs_t get_core    () noexcept;
    // task stack handling
    fmr::Local_int get_task_n () noexcept;
    fmr::Local_int add_task   (Work_spt) noexcept;// returns task number added
    fmr::Local_int del_task   (fmr::Local_int ix) noexcept;// returns task_n
  public:// pure virtual methods defined in CRTP base classes derived from Work
    virtual std::string get_base_abrv () noexcept =0;
    // intialize and exit
    virtual fmr::Exit_int init (int* argc, char** argv) noexcept =0;
    virtual fmr::Exit_int exit (fmr::Exit_int err=0)    noexcept =0;
  protected:
    std::string set_abrv (const std::string&) noexcept;
    // Work task_list initialization and exit
    fmr::Exit_int init_list (int* argc, char** argv) noexcept;// init forward
    fmr::Exit_int exit_list () noexcept;// exit task_list in reverse
    fmr::Exit_int exit_tree () noexcept;// exit task hierarchy in reverse
    // called by Derived::get_task_*(..)
    Work* get_work (fmr::Local_int) noexcept;
    Work* get_work (Work_type, fmr::Local_int ix=0) noexcept;
    Work* get_work (const Task_path_t&) noexcept;
  private:
    fmr::Exit_int exit_info (Work*, fmr::perf::Float busy_s);
  protected:// Built-ins called only by inherited classes ---------------------
    Work (Work::Core_ptrs_t) noexcept;// preferred constructor
    Work () noexcept;// default constructor for early parts of initialization
    Work (Work&&);              // movable
    Work& operator
      =  (const Work&) =delete; // not asignable
    Work (const Work&) =delete; // not copyable
    virtual ~Work ();// not inline to avoid -Winline large growth warning
  };// ========================================================================
}//end femera:: namespace

#include "Work.ipp"

#undef FMR_DEBUG
//end FEMERA_HAS_WORK_HPP
#endif
