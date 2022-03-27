#ifndef FEMERA_NODE_IPP
#define FEMERA_NODE_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  inline
  void proc::Node::task_init (int*, char**) {
#if 0
    this->set_auto ();                       //TODO move to proc::Root?
    //  const auto n = fmr::proc::get_node_n () * fmr::proc::get_node_core_n ()
    //    / this->all_proc_n ();
#endif
#if 0
    const auto core_n = this->node_n * this->get_core_n ();
    const auto  all_n = this->all_proc_n ();
    if (all_n < core_n) {//TODO Move this. It's too early here.
      printf ("Femera uses fewer threads (%u) than physical cores (%u)\n",
        all_n, core_n);
    }
    if (all_n > core_n) {
      printf ("Femera uses more threads (%u) than physical cores (%u)\n",
        all_n, core_n);
    }
#endif
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
