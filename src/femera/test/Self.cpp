#include "../core.h"
#include "Self.hpp"
#include "../proc/Node.hpp"

#include <valarray>

#if 0
//#include <gtest/gtest.h>
TEST( SelfTest, TrivialTest ){
  EXPECT_EQ( 0, 0 );
}
#endif
namespace femera {
  void test::Self::task_init (int*, char**) {// Test for initialization errors.
    const auto all_n = this->proc->all_proc_n ();
    const auto node_n = this->proc->get_task (Task_type::Node)->get_team_n ();
    const auto core_n = node_n * proc::Node::get_core_n ();
    const auto numa_n = node_n * proc::Node::get_numa_n ();
    const auto numa_all = node_n * numa_n;
    if (true) {//TODO detail && this->test->do_test
      const auto str = get_base_abrv ()+" "+ abrv +" ";
      this->log_init_info ();// pulled out method to prevent inline fail warning
      fmr::Local_int mpi_n=0, omp_n=0;
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
      this->data->name_line (data->fmrlog, str +"proc",
        "%4u    /%4u total CPU processes (%4u MPI  "+ x +"%4u OpenMP)",
        mpi_n * omp_n, all_n, mpi_n, omp_n);
      this->data->name_line (data->fmrlog, str +"node",
        "%4u    /%4u total physical CPUs (%4u node "+ x +"%4u cores)",
        node_n * core_n, all_n, node_n, core_n);
      this->data->name_line (data->fmrlog, str +"numa",
        "%4u ea "+ x +"%4u node:%4u NUMA domain%s total",
        numa_n, node_n, numa_all, (numa_all==1)?"":"s");
#endif
    }
    if (all_n < core_n) {
      this->data->name_line (data->fmrlog, this->abrv+" task_init",
        "NOTE Femera uses fewer threads (%u) than physical cores (%u).",
        all_n, core_n);
    }
    if (all_n > core_n) {
      this->data->name_line (data->fmrlog, this->abrv+" task_init",
        "NOTE Femera uses more threads (%u) than physical cores (%u).",
        all_n, core_n);
  } }
  void test::Self::log_init_info () {// pulled out of task_init (inline fail)
    const auto str = get_base_abrv ()+" "+ abrv +" ";
    const auto cppver = __cplusplus;
    this->data->name_line (data->fmrlog, str +" ver",
      std::string (MAKESTR(FMR_VERSION)));
#if 0
    this->data->name_line (data->fmrlog, str +" C++",
      std::to_string(cppver) +", gcc "+ std::string(__VERSION__));
#else
    this->data->name_line (data->fmrlog, str +" g++",
      std::string(__VERSION__));
    this->data->name_line (data->fmrlog, str +" C++", "%li", cppver);
#endif
    this->data->name_line (data->fmrlog, str +"zord",
      "%4u maximum hypercomplex order", zyclops::max_zorder);
  }
}//end femera namespace
