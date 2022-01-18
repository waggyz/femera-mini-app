#ifndef FEMERA_NODE_IPP
#define FEMERA_NODE_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  inline
  void proc::Node::task_init (int*, char**){}
  inline
  void proc::Node::task_exit () {
  }
  inline
  fmr::Local_int proc::Node::get_core_n () {// physical cores
    return fmr::proc::get_node_core_n ();
  }
  inline
  fmr::Local_int proc::Node::get_hype_n () {// physical+logical cores
    return fmr::proc::get_node_hype_n ();
  }
  inline
  fmr::Local_int proc::Node::get_core_ix () {
    return fmr::proc::get_node_core_ix ();
  }
  inline
  fmr::Local_int proc::Node::get_hype_ix () {
    return fmr::proc::get_node_hype_ix ();
  }
  inline
  fmr::Local_int proc::Node::get_numa_ix () {
    return fmr::proc::get_node_numa_ix ();
  }
  inline
  fmr::Local_int proc::Node::get_numa_n () {
    return fmr::proc::get_node_numa_n ();
  }
  inline
  fmr::Global_int proc::Node::get_used_byte () {
    return fmr::proc::get_used_byte ();
  }
}//end femera namespace

#undef FMR_DEBUG

//end FEMERA_NODE_IPP
#endif
