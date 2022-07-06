#include "../core.h"
#include "Node.hpp"

#include <unistd.h>
#include <limits.h>
#include <cstdio> 

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  proc::Node::Node (const femera::Work::Core_ptrs_t core)
  noexcept : Proc (core) {
    char hostname [HOST_NAME_MAX];
    const auto err = gethostname (hostname, HOST_NAME_MAX);
    if (err) { this->name = "node"; }
    else     { this->name = std::string (hostname); }
    this->abrv      ="node";
    this->task_type = task_cast (Task_type::Node);
    this->info_d    = 3;
  }
  void proc::Node::task_init (int*, char**) {
#if 0
    this->set_auto ();                       //TODO move to proc::Root?
    //  const auto n = fmr::proc::get_node_n () * fmr::proc::get_node_core_n ()
    //    / this->all_proc_n ();
#endif
  }
#if 0
  fmr::Local_int proc::Node::chck () {//TODO interface in File and Proc classes
    const auto all_n = 4;//this->proc->all_proc_n ();
//    const auto node_n = this->proc->get_task (Task_type::Node)->get_team_n ();
    const auto core_n = this->node_n * this->get_core_n ();
    const auto numa_n = this->node_n * this->get_numa_n ();
    const auto numa_all = this->node_n * numa_n;
FMR_WARN_INLINE_OFF
    if (true) {//TODO detail && this->test->do_test
      const auto str = this->get_base_abrv ()+" "+ this->abrv +" ";
      fmr::Local_int mpi_n=0, omp_n=0;
      //
      // segfaults here if called before tasks init
      const auto Pmpi = this->proc->get_task (Task_type::Fmpi);
      if (Pmpi) {mpi_n = Pmpi->get_proc_n ();}
      const auto Pomp = this->proc->get_task (Task_type::Fomp);
      if (Pomp) {omp_n = Pomp->get_proc_n ();}
#if 0
      this->data->name_line (data->fmrlog, str +"proc",
        "%4u MPI %4u OpenMP: %4u of %4u total CPU processes",
        mpi_n, omp_n, mpi_n * omp_n,  all_n);
      this->data->name_line (data->fmrlog, str +"node",
        "%4u node%4u cores : %4u of %4u total physical cores",
        node_n, core_n, node_n * core_n, all_n);
#else
#ifdef FMR_TIMES_UCHAR
      auto x = std::string ( MAKESTR(\FMR_TIMES_UCHAR) );
      x = (x =="\\x") ? "x" : x ;
#else
      const auto x = std::string ("x");
#endif
//      this->data->name_line (data->fmrlog, str +"proc",
      printf (
        ("%4u    /%4u total CPU processes (%4u MPI  "+ x +"%4u OpenMP)").c_str(),
        mpi_n * omp_n, all_n, mpi_n, omp_n);
//      this->data->name_line (data->fmrlog, str +"node",
      printf (
        ("%4u    /%4u total physical CPUs (%4u node "+ x +"%4u cores)").c_str(),
        node_n * core_n, all_n, node_n, core_n);
//      this->data->name_line (data->fmrlog, str +"numa",
      printf (
        ("%4u ea "+ x +"%4u node:%4u NUMA domain%s total").c_str(),
        numa_n, node_n, numa_all, (numa_all==1)?"":"s");
#endif
    }
    if (all_n < core_n) {
//      this->data->name_line (data->fmrlog, this->abrv+" task_init",
      printf (
        "NOTE Femera uses fewer threads (%u) than physical cores (%u).",
        all_n, core_n);
    }
    if (all_n > core_n) {
//      this->data->name_line (data->fmrlog, this->abrv+" task_init",
      printf (
        "NOTE Femera uses more threads (%u) than physical cores (%u).",
        all_n, core_n);
    }
FMR_WARN_INLINE_ON
  }
#endif
  void proc::Node::task_exit () {
    const auto use = fmr::form::si_unit (this->get_used_byte (), "B");
    const auto max = fmr::form::si_unit (this->get_dram_byte (), "B");
    const auto label = femera::form::text_line (80, "%4s %4s DRAM",
      this->get_base_abrv().c_str(), this->get_abrv ().c_str());
    femera::form::name_line (::stdout, 14, 80, label,
      "%s /%s maximum resident set size of this process",
      use.c_str(), max.c_str());
  }
}//end femera:: namespace
#undef FMR_DEBUG
