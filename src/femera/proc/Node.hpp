#ifndef FEMERA_NODE_HPP
#define FEMERA_NODE_HPP

#include "../Proc.hpp"

namespace femera { namespace proc {
  class Node;// Derive a CRTP concrete class from Proc.
  class Node : public Proc<Node> {private: friend class Proc;
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
    fmr::Local_int task_proc_ix ();
  private:
    Node (femera::Work::Core_ptrs) noexcept;
    Node () =delete;//NOTE Use the constructor above.
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::proc:: namespace

#include "Node.ipp"

//end FEMERA_NODE_HPP
#endif
