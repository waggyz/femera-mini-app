#ifndef FMR_HAS_PROC_HPP
#define FMR_HAS_PROC_HPP

#include "fmr.hpp"

namespace fmr { namespace proc {

  fmr::Local_int  get_node_core_n  ();
  fmr::Local_int  get_node_hype_n  ();
  fmr::Local_int  get_node_numa_n  ();
  fmr::Local_int  get_node_core_ix ();
  fmr::Local_int  get_node_hype_ix ();
  fmr::Local_int  get_node_numa_ix ();
  fmr::Global_int get_used_byte    ();
  fmr::Global_int get_dram_byte    ();

} }// end fmr::proc:: namespace

//#include "proc.ipp"

//end FMR_HAS_PROC_HPP
#endif
