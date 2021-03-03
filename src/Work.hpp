#ifndef FMR_HAS_WORK_HPP
#define FMR_HAS_WORK_HPP
/** */
#include "Task.hpp"

#include "perf.hpp"
#include "vals.hpp"

#include <string>
//#include <memory>     //TODO std::unique_ptr, std::make_unique

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
class Proc;// Forward-declare
class Data;
class Work {//TODO Refactor to the curiously recurrent template pattern (CRTP)
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
    int my_master = 0;
  private:
  // Methods ------------------------------------------------------------------
  public:
    Work           ()           =default;
    Work           (Work const&)=delete;// not copyable
    void operator= (const Work&)=delete;
    virtual ~Work  ();// virtual destructor for virtual class
    //
    int print_task_time (const std::string label_suffix);
    //
    // Work stack initialization and exit
    int init (int* argc, char** argv);
    int exit (int  err);
    //
    // Single-inheritance interface
    // These pure virtual functions must be specialized for each task.
    virtual int init_task (int* argc, char** argv)=0;
    virtual int exit_task (int  err)=0;
    //
    // If a derived class does not override these, then,
    // e.g., a call to Derived::prep() will call Work::prep().
    virtual int prep ();//TODO or virtual int init() ?
    virtual int chck ();//TODO chk?
    virtual std::string print_summary ();
    virtual std::string print_details ();
  protected:
  private:
};
}//end Femera namespace
#undef FMR_DEBUG
//end FMR_HAS_WORK_HPP
#endif
