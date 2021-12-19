#ifndef FMR_HAS_WORK_HPP
#define FMR_HAS_WORK_HPP
/** */
#include "../fmr/perf/Meter.hpp"
//#include "Task.hpp"
//#include "vals.hpp"

#include <string>
#include <memory>     //TODO std::shared_ptr, std::make_shared
#include <deque>

namespace femera {
// Forward-declares
//class Proc; class File; class Data; class Test;
namespace proc { class Base; using Base_t = std::shared_ptr <Base>; }
namespace data { class Base; using Base_t = std::shared_ptr <Base>; }
namespace file { class Base; using Base_t = std::shared_ptr <Base>; }
namespace test { class Base; using Base_t = std::shared_ptr <Base>;
                 class Perf; using Perf_t = std::shared_ptr <Perf>; }

// Classes derived from Work use the curiously recurrent template pattern (CRTP)
class Work;
using Work_t = std::shared_ptr <Work>;
class Work {// This is an abstract (pure virtual) base class (interface).
  public:// Variables ---------------------------------------------------------
    std::string      name = std::string ("unknown work");
    fmr::perf::Meter perf = fmr::perf::Meter ();
  protected:
    proc::Base_t proc = static_cast <proc::Base_t> (nullptr);// processing hier
    data::Base_t data = static_cast <data::Base_t> (nullptr);// data handling
    file::Base_t file = static_cast <file::Base_t> (nullptr);// file handling
    test::Base_t test = static_cast <test::Base_t> (nullptr);// correct and perf
  private:
    std::deque <Work_t> task_list = {};
    static const fmr::Dim_int info_d = 1;
  protected:// Methods --------------------------------------------------------
    inline Work_t get_work   (size_t) noexcept;
  public:
    //<Derived>_t get_task   (size_t) defined in derived and returns that type
    inline void   add_task   (Work_t) noexcept;
    inline size_t get_task_n ()       noexcept;
    //
    // Work stack initialization and exit
    virtual void init (int* argc, char** argv) =0;
    virtual int  exit (int err) =0;
    //
    inline Work (Work) noexcept;// preferred constructor
  public:// Built-in stuff ----------------------------------------------------
    Work ()           =default;
    Work (Work const&)=default;
    Work& operator=
         (const Work&)=default;
    virtual ~Work ()  =default;
};
}//end femera:: namespace

#include "Work.ipp"

#undef FMR_DEBUG
//end FMR_HAS_WORK_HPP
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
