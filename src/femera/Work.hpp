#ifndef FEMERA_HAS_WORK_HPP
#define FEMERA_HAS_WORK_HPP
/** */
#include "../fmr/perf/Meter.hpp"
//#include "Task.hpp"
//#include "vals.hpp"

#include <memory>     //TODO std::shared_ptr, std::make_shared
#include <string>
#include <vector>
#include <deque>
#include <exception>
#include <tuple>

namespace femera {
  // Forward-declares
  class Work;
  template <typename> class Proc;// class Main;
  template <typename> class Data;// class File;
  template <typename> class Test;// class Beds;
  template <typename> class Sims;
  namespace proc { class Main; class Ftop; class Fmpi; class Fomp; class Fcpu; }
  namespace data { class File; class Flog; }// class Type; class Base; }//TODO Fake -> Base?
  namespace test { class Beds; class Unit; class Self; class Perf; class Gtst; }
  namespace sims { class Jobs; }
  // typedefs
  using Work_t = std::shared_ptr <Work>;
  //
  using Main_t = std::shared_ptr <proc::Main>;// concrete Proc interface
  using Ftop_t = std::shared_ptr <proc::Ftop>;//FIXME should others be visible
  using Fmpi_t = std::shared_ptr <proc::Fmpi>;//      oustside of their parent
  using Fomp_t = std::shared_ptr <proc::Fomp>;//      class?
  using Fcpu_t = std::shared_ptr <proc::Fcpu>;
  //
  using File_t = std::shared_ptr <data::File>;// concrete Data interface
  using Flog_t = std::shared_ptr <data::Flog>;
  //
  using Beds_t = std::shared_ptr <test::Beds>;// concrete Test interface
  //using Unit_t = std::shared_ptr <test::Unit>;// built-in unit tests
  //using Self_t = std::shared_ptr <test::Self>;// built-in integration tests
  //using Perf_t = std::shared_ptr <test::Perf>;// built-in performance tests
  using Gtst_t = std::shared_ptr <test::Gtst>;
  //
  using Jobs_t = std::shared_ptr <sims::Jobs>;// concrete Sims interface
  //
  class Work {// This is an abstract (pure virtual) base class (interface).
  // Derived Classes use the curiously recurrent template pattern (CRTP).
  public:// typedefs ----------------------------------------------------------
    using Task_list_t = std::deque <Work_t>;
    using Task_path_t = std::vector<fmr::Local_int>;
    using Task_tree_t = std::vector<Task_path_t>;
//    using      Core_t = std::tuple <Main_t,File_t,Beds_t>;
    using      Core_t = std::tuple <proc::Main*,data::File*,test::Beds*>;
  public:// Variables ---------------------------------------------------------
    fmr::perf::Meter time = fmr::perf::Meter ();
    std::string      name = std::string      ("unknown work");
    //
    proc::Main* proc = nullptr;// processing hierarchy (proc::Main_t)
    data::File* data = nullptr;// data and file handling (data::File)
    test::Beds* test = nullptr;// correctness and performance testing {test::Beds}
  protected:
    Task_list_t task_list ={};
    fmr::Dim_int   info_d = 1;
  public:// Methods -----------------------------------------------------------
    //NOTE Make at least 1 method pure virtual.
    //FIXME Do all virtual methods need to be pure
    //      to avoid vtable using CRTP derived classes?
    virtual fmr::Exit_int init (int* argc, char** argv) noexcept =0;
    virtual fmr::Exit_int exit (fmr::Exit_int err=0) =0;
    //
//TODO    Task_tree_t   get_tree   () noexcept;
    fmr::Local_int add_task   (Work_t) noexcept;
    fmr::Local_int get_task_n ()       noexcept;
    //Derived_t    get_task   (fmr::Local_int) in Derived and returns that type
    //Derived_t    new_task   ()                     ""
    //Derived_t    new_task   (Core_t)               ""
    //
    Core_t get_core () noexcept;
  protected:
    Work_t get_work (fmr::Local_int) noexcept;
    Work_t get_work (Task_path_t)    noexcept;
    // above called by Derived::get_task(..)
    //
    // Work stack initialization and exit
    void          init_list (int* argc, char** argv) noexcept;// init forward
    fmr::Exit_int exit_list () noexcept;// exit task_list in reverse
    fmr::Exit_int exit_tree () noexcept;// exit task hierarchy in reverse
  protected:// Built-in stuff -------------------------------------------------
    // Make it clear this class needs to be inherited from.
    Work ()            =default;
    Work (Work const&) =default;// copyable
    Work& operator=
      (const Work&)    =default;
    virtual ~Work ();// not inline to avoid -Winline large growth warning
  };
  //
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
