#ifndef FMR_HAS_TYPE_HPP
#define FMR_HAS_TYPE_HPP

#include <string>
#include <typeinfo> // std::underlying_type
#include <map>
#include <vector>

#define FMR_TOUCH_VALS_FIRST

namespace fmr {
  /* These typedefs, enums, templates, and static maps can be used throughout
   * fmr:: and Femera:: namespaces.
   *
   * sizeof (Dim_int <= Enum_int <= Local_int < Global_int)
   * sizeof (Elid_int >= Enum_int + fmr::math::Poly + Dim_int)
   */
  typedef uint8_t     Dim_int;// dimensions, order, power, hierarchy lvls,...
  typedef uint16_t   Enum_int;
  typedef uint32_t  Local_int;
  typedef uint32_t   Elid_int;//(See above.)
  typedef uint64_t Global_int;//TODO signed?
  //
  typedef double   Geom_float;//TODO try float.
  typedef double   Phys_float;
  typedef double   Solv_float;
  typedef float    Cond_float;// Preconditioning and scaling
  typedef float    Post_float;// Post-processing
  typedef float    Plot_float;// Visualization
  //
  template <typename E>// Cast enum to number: for enum index#, size, sync.
  constexpr typename std::underlying_type<E>::type enum2val (E e) {
    //use: const auto part_type_n = fmr::enum2val (fmr::Partition::end);
    return static_cast<typename std::underlying_type<E>::type>(e);
  }
  enum class Vals_type : Enum_int { None=0, Error, Unknown,
    Dim, Enum, Local, Elid, Global,
    Geom, Phys, Solve, Cond, Post, Plot,
  end};
  enum class Partition : Enum_int { None=0, Error, Unknown,
    Prepared,// Pre-partitioned
        Join,// Treat as single partition (XS models)
        Mtrl,// One partition for each material
        Geom,// One partition for each element/cell type
        Elem,// Each partition is a sequential list of elements//TODO batch_sz
        Grid,// Partitioned by a structured grid
        Mesh,// Partitioned by a coarser mesh
        Auto,//TODO
      Plugin,
  end};
  //NOTE These string lookups do not need to be const (for compile-time
  //     evaluation) because they are only used for I/O operations. They could
  //     be std::array or std::unordered_map, but std::map syntax is cleaner.
  static const std::map<Partition,std::string> Partition_name {
    {Partition::     None,"unpartitioned"},
    {Partition::    Error,"partitioning error"},
    {Partition::  Unknown,"unknown partitioning"},
    {Partition:: Prepared,"pre-partitioned"},
    {Partition::     Join,"merged"},
    {Partition::     Mtrl,"partition by material"},
    {Partition::     Geom,"partition by element type"},
    {Partition::     Elem,"partition by element id"},//TODO needs bin_size
    {Partition::     Grid,"grid-partitioned"},
    {Partition::     Mesh,"mesh-partitioned"},
    {Partition::     Auto,"partition automatically"},
    {Partition::   Plugin,"partitioner plugin"}
  };
  enum class Schedule : Enum_int { None=0, Error, Unknown,
    Once,
    List, Block, Interleave, Fifo, Filo,
    Auto,
    Plugin,
  end};
  //TODO Batches: distribute batch_sz objects at a time.
  //     Objects in a batch are run in a Serial List.
  static const std::map<Schedule,std::string> Schedule_name {
    {Schedule::       None,"no schedule"},
    {Schedule::      Error,"schedule error"},
    {Schedule::    Unknown,"unknown schedule"},
    {Schedule::       Once,"alone"},// Sched. one object to run on one thread.*
    {Schedule::       List,"preset list"},
    {Schedule::      Block,"block list"},
    {Schedule:: Interleave,"interleaved list"},
    {Schedule::       Fifo,"first in/first out"},
    {Schedule::       Filo,"first in/last out"},
    {Schedule::       Auto,"auto-schedule"},
    {Schedule::     Plugin,"scheduler plugin"}
    // * Once: batch size = 1
  };
  enum class Concurrency : Enum_int { None=0, Error, Unknown,
    Once,       // Distribute one object to one thread.
    Serial,     // Distribute and run all objects only on master thread.
    Independent,// Distribute and run different objs. on each lower thread.
    Collective, // Distribute and run the same object on all lower threads.
  end};
  static const std::map<Concurrency,std::string> Concurrency_name {
    {Concurrency::        None,"no concurrency"},//TODO makes sense?
    {Concurrency::       Error,"concurrency error"},
    {Concurrency::     Unknown,"unknown concurrency"},
    {Concurrency::        Once,"singly"},
    {Concurrency::      Serial,"serially"},
    {Concurrency:: Independent,"independently"},
    {Concurrency::  Collective,"collectively"}
  };
  struct Distribute {
    fmr::Local_int   bats_sz = 1;
    fmr::Schedule    plan    = fmr::Schedule::Once;
    fmr::Concurrency cncr    = fmr::Concurrency::Once;
    fmr::Dim_int     hier_lv = 1;// processing hierarchy run level
    // constructor
    Distribute (const fmr::Dim_int h, const fmr::Schedule p,
      const fmr::Concurrency c, fmr::Local_int b=1) noexcept
      : bats_sz(b), plan(p), cncr(c), hier_lv(h){}
  };
  enum class Sim_time : Enum_int { None=0, Error, Unknown, Plugin,
    Explicit,// Time-accurate
    Implicit,// Not time-accurate
  end};
  static const std::map<Sim_time,std::string> Sim_time_name {
    {Sim_time::     None,"no sim time type"},//TODO makes sense?
    {Sim_time::    Error,"sim time type error"},
    {Sim_time::  Unknown,"unknown sim time type"},
    {Sim_time::   Plugin,"plugin"},
    {Sim_time:: Explicit,"explicit"},
    {Sim_time:: Implicit,"implicit"}
  };
  enum class Sim_size :fmr::Enum_int{ None=0, Error, Unknown, Plugin,
    Automatic,
    XS, SM, MD, LG, XL
  };
  static const std::map<Sim_size,std::string> Sim_size_name {
    {Sim_size::      None,"no sims"},//TODO makes sense?
    {Sim_size::     Error,"sim size error"},
    {Sim_size::   Unknown,"unknown sim size"},
    {Sim_size::    Plugin,"plugin sim size handler"},
    {Sim_size:: Automatic,"automatic sim size handler"},
    {Sim_size::        XS,"extra small"},
    {Sim_size::        SM,"small"},
    {Sim_size::        MD,"medium"},
    {Sim_size::        LG,"large"},
    {Sim_size::        XL,"extra large"}
  };
  static const std::map<Sim_size,std::string> Sim_size_short {
    {Sim_size::      None,"none"},//TODO makes sense?
    {Sim_size::     Error,"erro"},
    {Sim_size::   Unknown,"unknown"},
    {Sim_size::    Plugin,"plug"},
    {Sim_size:: Automatic,"auto"},
    {Sim_size::        XS,"XS"},
    {Sim_size::        SM,"SM"},
    {Sim_size::        MD,"MD"},
    {Sim_size::        LG,"LG"},
    {Sim_size::        XL,"XL"}
  };
#if 0
  enum class Distribute :fmr::Enum_int{ Unknown=-1, None=0, Automatic,
    Master, Here, One_per_core, To_all_cores,
    By_size, XS, SM, MD, LG, XL
  };
#endif
  template <typename T> static const std::string get_enum_string
    (const std::map<T,std::string> name_map, const T val) {
    if (name_map.count(val) > 0) {return name_map.at(val);}
    return "unnamed item "+std::to_string (fmr::enum2val (val));
  }
#if 0
  template <typename T> static const std::string get_enum_string
   (const std::vector<std::string> name_vec, const T val) {
    const size_t v = enum2val (val);
    if (v>=0 && v<name_vec.size()) {return name_vec[v];}
    return "unnamed item "+std::to_string (v);
  }
#endif
  template <typename T, typename C> static const std::string get_enum_string
   (const C name_container, const T val) {
    const size_t v = enum2val (val);
    if (v>=0 && v<name_container.size()) {
      return name_container[v];
    }
    return "unnamed item "+std::to_string (v);
  }
}//end fmr:: namespace
namespace fmr {namespace data {
  enum class Access : fmr::Enum_int {Unknown=0, Error,//TODO Find, Scan, Append?
    New, Check, Read, Write, Modify, Close
  };
  static const std::map<Access,std::string> Access_name {
    {Access:: Unknown,"unknown access"},//TODO makes sense?
    {Access::   Error,"access error"},
    {Access::     New,"new"},
    {Access::   Check,"check"},
    {Access::    Read,"read"},
    {Access::   Write,"write"},
    {Access::  Modify,"modify"},
    {Access::   Close,"close"}
  };
} }//end fmr::data:: namespace

#if 0
  //https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
  struct int_enum_class_hash {//May not be needed for std::map *_name above.[1]
    template <typename T> int operator()(T t) const {
      return static_cast<int>(t);
    }
  //[1] "This was considered a defect in the standard, and was fixed in C++14"
  };
#endif
// end FMR_HAS_TYPE_HPP
#endif
