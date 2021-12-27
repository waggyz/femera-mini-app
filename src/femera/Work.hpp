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
  template <typename> class Proc;
  template <typename> class Data;
  template <typename> class Test;
  namespace proc { class Main; class Fcpu; class Fomp; class Fmpi; }
  namespace data { class File; class Flog; }// class Type; class Base; }//FIXME Fake -> Base?
  namespace test { class Beds; class Unit; class Self; class Perf; class Gtst; }

  using Work_t = std::shared_ptr <Work>;
  using Proc_t = std::shared_ptr <Proc<Work>>;
  using Main_t = std::shared_ptr <proc::Main>;
  using Fcpu_t = std::shared_ptr <proc::Fcpu>;
  using Fomp_t = std::shared_ptr <proc::Fomp>;
  using Fmpi_t = std::shared_ptr <proc::Fmpi>;
#if 0
  template <typename> File;
  namespace file { Flog; }// class Exts;
  using File_t = std::shared_ptr <File<Work>>;
  using Flog_t = std::shared_ptr <file::Flog>;
  using Exts_t = std::shared_ptr <file::Exts>;
  using Data_t = std::shared_ptr <Data<Work>>;
  using Type_t = std::shared_ptr <data::Type>;
#endif
  using Data_t = std::shared_ptr <Data<Work>>;
  using File_t = std::shared_ptr <data::File>;
  using Flog_t = std::shared_ptr <data::Flog>;

  using Test_t = std::shared_ptr <Test<Work>>;
  using Beds_t = std::shared_ptr <test::Beds>;
  //using Unit_t = std::shared_ptr <test::Unit>;
  //using Self_t = std::shared_ptr <test::Self>;
  //using Perf_t = std::shared_ptr <test::Perf>;
  using Gtst_t = std::shared_ptr <test::Gtst>;

//FIXME is, e.g., class femera::Proc_base or femera::Proc::Base better?

  class Work {// This is an abstract (pure virtual) base class (interface).
  // Derived Classes use the curiously recurrent template pattern (CRTP).
  public:
    using Task_list_t = std::deque <Work_t>;
//    using Make_work_t = std::tuple <Proc_t,File_t,Data_t,Test_t>;
    using Make_work_t = std::tuple <Main_t,File_t,Beds_t>;
  public:// Variables ---------------------------------------------------------
    fmr::perf::Meter time = fmr::perf::Meter ();
    std::string      name = std::string      ("unknown work");
    //
    //Proc_t proc = static_cast <Proc_t> (nullptr);
    Main_t proc = nullptr;// processing hierarchy (Main_t)
    File_t data = nullptr;// file handling
//    Flog_t file = nullptr;// file handling
//    Type_t data = nullptr;// data handling
    Beds_t test = nullptr;// correctness and performance testing
  private:
    Task_list_t task_list ={};
    fmr::Dim_int   info_d = 1;
  protected:// Methods --------------------------------------------------------
    //NOTE Make at least 1 pure virtual.
    Work_t get_work   (size_t) noexcept;// used by derived get_task(..) method
    Work_t get_work   (std::vector<size_t>) noexcept;// ""
  public:
    // Derived_t get_task (size_t) is in Derived class and returns that type.
    size_t add_task   (Work_t) noexcept;
    size_t get_task_n ()       noexcept;
    //
    // Work stack initialization and exit
    virtual void          init (int* argc, char** argv) =0;
    virtual fmr::Exit_int exit (fmr::Exit_int err=0) noexcept;
    // default is to exit task_list in reverse
    //
    Make_work_t ptrs () noexcept;
  public:// Built-in stuff ----------------------------------------------------
    Work ()            =default;
    Work (Work const&) =default;// copyable
    Work& operator=
      (const Work&)    =default;
    virtual ~Work ()   =default;
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
