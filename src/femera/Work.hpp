#ifndef FMR_HAS_WORK_HPP
#define FMR_HAS_WORK_HPP
/** */
//#include "Task.hpp"
//#include "perf.hpp"
//#include "vals.hpp"

#include <string>
#include <memory>     //TODO std::shared_ptr, std::make_shared

#include <deque>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif


namespace Femera {

template <class T>// Work uses the curiously recurrent template pattern (CRTP)
class Work {// This is an abstract (pure virtual) class (interface).
  // Variables ----------------------------------------------------------------
  public:
    std::string name ="unknown work";
    // public because parent accesses derived instance variables
    std::shared_ptr <Proc::Base> proc = nullptr;// Processing hierarchy
    std::shared_ptr <Data::Base> data = nullptr;// Data handling
    std::shared_ptr <File::Base> file = nullptr;// File handling
    std::shared_ptr <Test::Base> test = nullptr;// Correctness and perf testing
    //
    // Task<Work>  task = Task<Work>(this);
    std::deque <std::shared_ptr<T>> task_list = {};
    //
    fmr::perf::Meter perf = fmr::perf::Meter();
    //
    int info_d = 1;
  // Methods ------------------------------------------------------------------
  public:
    // Work stack initialization and exit
//    virtual void init (int* argc, char** argv)=0;
//    virtual int  exit (int  ierr)=0;
    //
    Work (int* argc, char** argv)=0;
    Work (Work*);
    //
    Work           ()           =default;
    Work           (Work const&)=delete;// not copyable
    void operator= (const Work&)=delete;
    virtual ~Work  ()=0;// pure virtual destructor indicates Work is abstract
};
class Proc; class File; class Data; class Test;// Forward-declares
class Proc::Base; class File::Base; class Data::Base; class Test::Base;
class Test::Perf;
//
class Main : Work<Main> {// Has top-level tasks
  // Variables ----------------------------------------------------------------
  public:
    std::string name ="Main";
  // Methods ------------------------------------------------------------------
  public:
    Main (int* argc, char** argv);
    //
    Main ()                     =default;
    Main (Main const&)          =delete;// not copyable
    void operator= (const Main&)=delete;
    ~Main ()                    =default;
};
}//end Femera namespace
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
