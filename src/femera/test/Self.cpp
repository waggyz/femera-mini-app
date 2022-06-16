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
    if (true) {//TODO detail && this->test->do_test
      fmr::Local_int mpi_n=0, omp_n=0;
      const auto Pmpi = this->proc->get_task (Task_type::Fmpi);
      if (Pmpi) {mpi_n = Pmpi->get_proc_n ();}
      const auto Pomp = this->proc->get_task (Task_type::Fomp);
      if (Pomp) {omp_n = Pomp->get_proc_n ();}
      this->data->name_line (data->fmrlog, get_base_abrv ()+" "+ abrv +" proc",
        "%4u MPI %4u OpenMP = %4u of %4u total CPU processes",
        mpi_n, omp_n, mpi_n * omp_n,  all_n);
      this->data->name_line (data->fmrlog, get_base_abrv ()+" "+ abrv +" node",
        "%4u node%4u cores  = %4u of %4u total physical cores",
        node_n, core_n, node_n * core_n, all_n);
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
}//end femera namespace
