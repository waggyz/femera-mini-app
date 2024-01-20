#include "../core.h"
#include "Self.hpp"

//TODO Remove Node.hpp" below?
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
    this->log_init_info ();// pulled out method to prevent inline fail warning
    this->proc_node_chck ();
    this->set_init (true);
  }
  void test::Self::log_init_info () {// pulled out of task_init (inline fail)
    this->data->send (fmr::info, this->get_base_abrv (), this->abrv, "fmr",
      std::string (MAKESTR(FMR_VERSION)));
    this->data->send (fmr::info, this->get_base_abrv (), this->abrv, "g++",
      std::string (__VERSION__));
    const auto cppver = __cplusplus;
    this->data->send (fmr::info, this->get_base_abrv (), this->abrv, "C++",
      "%li", cppver);
    this->data->send (fmr::info, this->get_base_abrv (), this->abrv, "zord",
      "%4u maximum hypercomplex order", zyclops::max_zorder);
  }
  fmr::Exit_int test::Self::proc_node_chck () {
    fmr::Exit_int err = 0;
    const auto all_n = this->proc->all_proc_n ();
    const auto node_n = this->proc->get_task (Task_type::Node)->get_team_n ();
    const auto core_n = node_n * proc::Node::get_core_n ();
    const auto numa_n = node_n * proc::Node::get_numa_n ();
    const auto numa_all = node_n * numa_n;
    if (true) {//TODO if this->test->do_test ?
      fmr::Local_int mpi_n=0, omp_n=0;
      //
      // segfaults here if called before tasks init
      const auto Pmpi = this->proc->get_task (Task_type::Fmpi);
      if (Pmpi != nullptr) { mpi_n = Pmpi->get_proc_n (); }
      const auto Pomp = this->proc->get_task (Task_type::Fomp);
      if (Pomp != nullptr) { omp_n = Pomp->get_proc_n (); }
#ifdef FMR_TIMES_UCHAR
      auto x = std::string ( MAKESTR(\FMR_TIMES_UCHAR) );
      x = (x =="\\x") ? "x" : x ;
#else
      const auto x = std::string ("x");
#endif
      this->data->send (fmr::log, get_base_abrv (), abrv, "proc",
        ("%4u    /%4u total CPU processes (%4u MPI  "
          + x +"%4u OpenMP)").c_str(),
        mpi_n * omp_n, all_n, mpi_n, omp_n);
      this->data->send (fmr::log, get_base_abrv (), abrv, "core",
        ("%4u    /%4u total physical CPUs (%4u node "
          + x +"%4u cores )").c_str(),
        node_n * core_n, all_n, node_n, core_n);
      this->data->send (fmr::log, get_base_abrv (), abrv, "numa",
        ("%4u ea "+ x +"%4u node,%4u NUMA domain%s total").c_str(),
        numa_n, node_n, numa_all, (numa_all==1)?"":"s");
    }
    if (all_n < core_n) { err =-1;
      this->data->send (fmr::log, get_base_abrv (), abrv, "NOTE",
        "Femera uses fewer threads (%u) than physical cores (%u).",
        all_n, core_n);
    }
    if (all_n > core_n) { err =-1;
      this->data->send (fmr::log, get_base_abrv (), abrv, "NOTE",
        "Femera uses more threads (%u) than physical cores (%u).",
        all_n, core_n);
    }
  return err;
  }
}//end femera namespace
