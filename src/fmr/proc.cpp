#include "proc.hpp"

#include <thread>     // hardware_concurrency (ncpu)
#include <sys/resource.h> //rusage

#ifdef FMR_HAS_LIBNUMA
#include <numa.h>     // numa_node_of_cpu, numa_num_configured_nodes
#endif

namespace fmr {
  fmr::Local_int proc::get_node_core_n () {
#ifdef FMR_CORE_N
    return FMR_CORE_N ;
#else
    return proc::get_node_hype_n ();
#endif
  }
  fmr::Local_int proc::get_node_hype_n () {//NOTE physical+logical cores
    return fmr::Local_int (std::thread::hardware_concurrency());
  }
  fmr::Local_int proc::get_node_core_ix () {
    return fmr::Local_int (fmr::Local_int (::sched_getcpu())
      % proc::get_node_core_n ());
  }
  fmr::Local_int proc::get_node_hype_ix () {
    return fmr::Local_int (fmr::Local_int (::sched_getcpu()));
  }
#ifdef FMR_HAS_LIBNUMA
  fmr::Local_int proc::get_node_numa_ix () {
    return (::numa_available() == -1) ? 0
      : fmr::Local_int (::numa_node_of_cpu (::sched_getcpu()));
  }
  fmr::Local_int proc::get_node_numa_n () {
    return (::numa_available() == -1) ? 1
      : fmr::Local_int (::numa_num_configured_nodes());
  }
#else
  fmr::Local_int proc::get_node_numa_ix () {
    return fmr::Local_int (0);
  }
  fmr::Local_int proc::get_node_numa_n () {
    return fmr::Local_int (1);
  }
#endif
  fmr::Global_int proc::get_used_byte () {
    rusage r;
    getrusage (RUSAGE_SELF, & r);
    return fmr::Global_int (r.ru_maxrss * 1024);
  }
}// end fmr:: namespace
