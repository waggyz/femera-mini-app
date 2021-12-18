#ifndef FMR_HAS_WORK_HPP
#define FMR_HAS_WORK_HPP
/** */
#include "../fmr/perf/Meter.hpp"
//#include "Task.hpp"
//#include "vals.hpp"

#include <string>
#include <memory>     //TODO std::shared_ptr, std::make_shared
#include <deque>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif


namespace femera {
// Forward-declares
//class Proc; class File; class Data; class Test;
namespace proc {class Base;}
namespace file {class Base;}
namespace data {class Base;}
namespace test {class Base; class Perf;}
// Work uses the curiously recurrent template pattern (CRTP)

//template <class T>
class Work {// This is an abstract (pure virtual) base class (interface).
  public:// Variables ---------------------------------------------------------
    std::string name = std::string("unknown work");
    // public because parent accesses derived instance variables
    std::shared_ptr <proc::Base> proc // Processing hierarchy
      = static_cast <std::shared_ptr <proc::Base>> (nullptr);
    std::shared_ptr <data::Base> data // Data handling
      = static_cast <std::shared_ptr <data::Base>> (nullptr);
    std::shared_ptr <file::Base> file // File handling
      = static_cast <std::shared_ptr <file::Base>> (nullptr);
    std::shared_ptr <test::Base> test // Correctness and perf testing
      = static_cast <std::shared_ptr <test::Base>> (nullptr);
    //
    fmr::perf::Meter perf = fmr::perf::Meter();
  private:
    // Task<Work>  task = Task<Work>(this);
    //std::deque <std::shared_ptr<T>> task_list = {};
    std::deque <std::shared_ptr<Work>> task_list = {};
    static const fmr::Dim_int info_d = 1;
  public:// Methods -----------------------------------------------------------
    // Work stack initialization and exit
    virtual void init (int* argc, char** argv)=0;
    virtual int  exit (int err)=0;
    void add_task (std::shared_ptr<Work> W) {task_list.push_back(W);};
    size_t get_task_n (){return task_list.size();};
    std::shared_ptr<Work> get_task (int i){return task_list[i];}
    //
    Work (Work*) noexcept;
    //
    Work           ()           =default;
    Work           (Work const&)=default;
    Work& operator= (const Work&)=default;
    //Work           (Work const&)=delete;// not copyable
    //void operator= (const Work&)=delete;
    virtual ~Work  ()           =default;
};
}//end femera:: namespace
#include "Work.ipp"

#if 0
namespace femera { namespace work {
class Base : Work <Base> {// Has top-level tasks
  // Variables ----------------------------------------------------------------
  public:
    std::string name ="Work base";
  // Methods ------------------------------------------------------------------
  public:
//    Base (int* argc, char** argv){};
    //
    Base ()                     =default;
    Base (Base const&)          =delete;// not copyable
    void operator= (const Base&)=delete;
    ~Base ()                    =default;
};
} }//end femera::work:: namespace
#endif

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
