#ifndef FMR_HAS_TYPE_HPP
#define FMR_HAS_TYPE_HPP

#include <typeinfo> // std::underlying_type
#include <vector>
#include <map>

#define FMR_TOUCH_VALS_FIRST

namespace fmr {
  /* These typedefs, enums, and static maps are used throughout
   * fmr:: and Femera:: namespaces.
   */
  // sizeof ( Dim_int <= Enum_int <= Local_int < Global_int )
  // sizeof ( Dim_int <~ int ): relative (negative) Dim_int are int parameters
  typedef uint8_t      Dim_int;// dimensions, order, power, hierarchy lvls,...
  typedef uint16_t    Enum_int;
  typedef uint32_t   Local_int;
  typedef uint64_t  Global_int;//TODO signed?
  typedef uint32_t Elem_id_int;
  //NOTE sizeof ( Elem_id_int >= Enum_int + fmr::math::Poly + Dim_int )
  //
  typedef double   Geom_float;
  typedef double   Phys_float;
  typedef double   Solv_float;
  typedef float    Cond_float;
  //
  enum class Partition : Enum_int { None=0, Error, Unknown,
    Prepared,// Pre-partitioned
       Merge,// Treat as single partitions, 1/elem type (XS models)
        Mtrl,// One partition for each material
        Geom,// One partition for each element/cell type
        Elem,// Each partition is a sequential list of elements//TODO batch_sz
        Grid,// Partition by a structured grid
        Mesh,// Partition by a coarser mesh
        Auto,//TODO
      Plugin
  };
  //NOTE These string lookups do not need to be const for compile-time use
  //     because they are only used for I/O operations. They could be
  //     std::array, but std::map syntax is cleaner.
  static const std::map<Partition,std::string> Partition_name {
    {Partition::      None,"unpartitioned"},
    {Partition::     Error,"partitioning error"},
    {Partition::   Unknown,"unknown partitioning"},
    {Partition::  Prepared,"pre-partitioned"},
    {Partition::     Merge,"merged"},
    {Partition::      Mtrl,"partition by material"},
    {Partition::      Geom,"partition by element type"},
    {Partition::      Elem,"partition by element id"},//TODO needs bin_size
    {Partition::      Grid,"grid-partitioned"},
    {Partition::      Mesh,"mesh-partitioned"},
    {Partition::      Auto,"partition automatically"},
    {Partition::    Plugin,"partitioner plugin"}
  };
  enum class Schedule : Enum_int { None=0, Error, Unknown,
    Once, List, Fifo,
    Auto,
    Plugin
  };
  //TODO Batches: distribute batch_sz objects at a time.
  //     Objects in a batch are run in a Serial List.
  static const std::map<Schedule,std::string> Schedule_name {
    {Schedule::    None,"no schedule"},
    {Schedule::   Error,"schedule error"},
    {Schedule:: Unknown,"unknown schedule"},
    {Schedule::    Once,"alone"},// Sched. one object to run on one thread.*
    {Schedule::    List,"preset schedule"},//TODO Close and Spread
    {Schedule::    Fifo,"first in/first out"},
    {Schedule::    Auto,"auto-schedule"},
    {Schedule::  Plugin,"scheduler plugin"}
    // * Once: batch size = 1
  };
   enum class Concurrency : Enum_int { None=0, Error, Unknown,
    Once,       // Distribute one object to one thread.
    Serial,     // Distribute and run all objects only on master thread.
    Independent,// Distribute and run different objs. on each lower thread.
    Collective  // Distribute and run the same object on all lower threads.
  };
  static const std::map<Concurrency,std::string> Concurrency_name {
    {Concurrency::        None,"no concurrency"},//TODO makes sense?
    {Concurrency::       Error,"concurrency error"},
    {Concurrency::     Unknown,"unknown concurrency"},
    {Concurrency::        Once,"singly"},
    {Concurrency::      Serial,"serially"},
    {Concurrency:: Independent,"independently"},
    {Concurrency::  Collective,"collectively"}
  };
   enum class Sim_time : Enum_int { None=0, Error, Unknown, Plugin,
    Explicit,// Time-accurate
    Implicit // Not time-accurate
  };
  static const std::map<Sim_time,std::string> Sim_time_name {
    {Sim_time::     None,"no sim type"},//TODO makes sense?
    {Sim_time::    Error,"sim type error"},
    {Sim_time::  Unknown,"unknown sim type"},
    {Sim_time::   Plugin,"plugin"},
    {Sim_time:: Explicit,"explicit"},
    {Sim_time:: Implicit,"implicit"}
  };
  //
  template <typename E>// Cast enum to integer value: for indexing and sizing.
  constexpr typename std::underlying_type<E>::type enum2val (E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
  }
#if 0
  enum class Distribute :fmr::Enum_int{ Unknown=-1, None=0, Automatic,
    Master, Here, One_per_core, To_all_cores,
    By_size, XS, SM, MD, LG, XL
  };
#endif
}//end fmr:: namespace
#if 0
namespace fmr {namespace data {//TODO Fix and remove from Data.hpp
  enum class Access : fmr::Enum_int {Unknown=0, Error,//TODO remove from Data.hpp
    New, Check, Read, Write, Modify, Close            //     Femera::Data::Access
  };//TODO Find, Scan?
  static const std::map<Access,std::string> Access_name {
    {Access:: Unknown,"uknown access"},//TODO makes sense?
    {Access::   Error,"access error"},
    {Access::     New,"new"},
    {Access::   Check,"check"},
    {Access::    Read,"read"},
    {Access::   Write,"write"},
    {Access::  Modify,"modify"},
    {Access::   Close,"close"}
  };
} }//end fmr::data:: namespace
#endif
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
