#ifndef FEMERA_NODE_HPP
#define FEMERA_NODE_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Node;// Derive a CRTP concrete class from Proc.
  class Node final: public Proc<Node> { friend class Proc; friend class Main;
  private:
    fmr::Local_int node_n = 1;
  public:
#if 0
    static  fmr::Local_int get_node_n    ();
    static  fmr::Local_int get_node_ix   ();
#endif
    static  fmr::Local_int get_core_n    ();
    static  fmr::Local_int get_hype_n    ();
    static  fmr::Local_int get_numa_n    ();
    static  fmr::Local_int get_core_ix   ();
    static  fmr::Local_int get_hype_ix   ();
    static  fmr::Local_int get_numa_ix   ();
    static fmr::Global_int get_used_byte ();
    static fmr::Global_int get_dram_byte ();
#if 0
    fmr::Local_int task_proc_ix ();
#endif
  private:
    Node (femera::Work::Core_ptrs_t) noexcept;
    Node () = delete;//NOTE Use the constructor above.
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Node.ipp"

//end FEMERA_NODE_HPP
#endif
