#ifndef FEMERA_HAS_WORK_HPP
#define FEMERA_HAS_WORK_HPP
/** */
#include "../fmr/perf/Meter.hpp"
#include "../fmr/math.hpp"
#include "Errs.hpp"
#include "type.hpp"
//#include "vals.hpp"

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
//#define FMR_MAKE_SMART std::make_shared
#define FMR_MAKE_SMART(T) std::make_shared<T>
#endif

namespace femera {
  // Forward declares
  class Work;
  template <typename> class Proc;// class Main; interface
  template <typename> class Data;// class File; interface
  template <typename> class Test;// class Beds; interface
  template <typename> class Task;// class Jobs; interface
  //
  namespace proc {// CRTP inheriance from Proc; interface is Main
    class Main; class Root; class Node; class Fcpu;
    class Fmpi; class Fomp; class Nvid;
  }
  namespace data {// CRTP inheriance from Data; interface is File
    class File; class Logs; class Dlim; class Text;
    class Cgns; class Gmsh; class Moab; class Moab; class Pets;
  }
  namespace test {// CRTP inheriance from Test; interface is Beds
    class Beds; class Unit; class Self; class Perf;
    class Gtst;
  }
  namespace task {// CRTP inheriance from Task; interface is Task
    class Jobs; class Sims; class Runs;// Post
  }
  // typedefs
  using Work_spt = FMR_SMART_PTR <Work>;
  using Jobs_spt = FMR_SMART_PTR <task::Jobs>;// concrete Task interface
  //
  //
  class Work {/* This is an abstract (pure virtual) base class (interface).
  * Derived classes use the curiously recurrent template pattern (CRTP) e.g.,
  * class Proc : public Work { .. };
  * class Main : public Proc <Main> { private: friend class Proc; .. };
  */
  public:// typedefs ==========================================================
    using Core_ptrs_t = std::tuple <proc::Main*, data::File*, test::Beds*>;
    using Task_path_t = std::vector <fmr::Local_int>;
    using Work_time_t = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>;
//TODO    using Task_tree_t = std::vector <Task_path_t>;
  public:// variables are classes with (mostly) only member functions public --
    Work_time_t time = Work_time_t ();// performance timer
    proc::Main* proc = nullptr;       // processing hierarchy
    data::File* data = nullptr;       // data, logging, and file handling
    test::Beds* test = nullptr;       // correctness and performance testing
  public:// methods -----------------------------------------------------------
    template <typename T, typename C> static inline constexpr
    T* cast_via_work (C* child)
    noexcept;
    std::string    set_name (const std::string&) noexcept;
    std::string    get_name    ()         noexcept;
    std::string    get_abrv    ()         noexcept;
    std::string    get_version ()         noexcept;
    Core_ptrs_t    get_core    ()         noexcept;
    fmr::Local_int get_task_n  ()         noexcept;
//TODO Task_tree_t get_tree    ()         noexcept;
    fmr::Local_int add_task    (Work_spt) noexcept;// returns task number added
    fmr::Local_int del_task    (fmr::Local_int ix) noexcept;// returns task_n
  public:// virtual methods ---------------------------------------------------
    virtual std::string get_base_abrv () noexcept =0;
    // intialize and exit
    virtual fmr::Exit_int init (int* argc, char** argv) noexcept =0;
    virtual fmr::Exit_int exit (fmr::Exit_int err=0)    noexcept =0;
  private:// ==================================================================
    using Task_list_t = std::deque <Work_spt>;
  protected:
    std::string       name ="unknown work";
    std::string       abrv ="work";
    std::string    version ="";
    Task_list_t  task_list ={};
    Work_type    task_type = task_cast (Base_type::Work);
    fmr::Dim_int info_d    = 1;
#if 0
// https://stackoverflow.com/questions/60040665
    /replacing-the-command-line-arguments-int-argc-and-char-argv-with-stdvectors
    //
    std::string argv_prfx ="-fmr:";
    std::unique_ptr<char*[]> my_argv;// use: err= init (&my_argc, my_argv.get());
    int                      my_argc =0;
#endif
  private:
    bool did_work_init = false;
    bool  is_work_main = true ;// save for use after proc::exit (..)
  protected:
    std::string set_abrv (const std::string&) noexcept;
  protected:// called by Derived::get_task_*(..)
    Work* get_work_raw (fmr::Local_int) noexcept;//TODO Change to get_work(..)
    Work* get_work_raw (const Task_path_t&) noexcept;//              ""
    Work* get_work_raw (Work_type, fmr::Local_int ix=0) noexcept;//  ""
#if 0
    Work_spt get_work_spt (fmr::Local_int) noexcept;
    Work_spt get_work_spt (Task_path_t)    noexcept;
#endif
#if 0
  public:
    template <typename T>
    T* get_task (fmr::Local_int ix=0) {//return task #ix of specific type
      return static_cast<T*>(this->get_work_raw(ix));}
#endif
  protected:
    // Work stack initialization and exit
    fmr::Exit_int init_list (int* argc, char** argv) noexcept;// init forward
    fmr::Exit_int exit_list () noexcept;// exit task_list in reverse
    fmr::Exit_int exit_tree () noexcept;// exit task hierarchy in reverse
  protected:// Built-in stuff -------------------------------------------------
    Work (Work::Core_ptrs_t) noexcept;
    Work ()            =default;
    Work (Work const&);// copyable, default constructor in Work.cpp
    Work& operator =
      (const Work&)    =default;
  public:
    virtual ~Work ();// not inline to avoid -Winline large growth warning
  };
}//end femera:: namespace

#include "Work.ipp"

#undef FMR_DEBUG
//end FEMERA_HAS_WORK_HPP
#endif
