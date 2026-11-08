#include "Bars.perf.hpp"
#include "../../femera.hpp"
#include "Bars.hpp"

#include <gtest/gtest.h>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}
using Perf_time_t = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>;

namespace femera { namespace grid {

  void elem::send_perf_results (std::string label, int test_case,
  fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>& perf,
  uint test_count, uint check) {
    const auto this_time = perf.get_busy_s ();
    mini->data->send (fmr::out, "perf", "Elem", label.c_str(),
      "(%i) %s: %s/%s test: %s",
      test_case,
      fmr::form::si_unit (perf.get_busy_unit_speed (), "el  /s").c_str(),
      fmr::form::si_unit (perf.get_unit (), "el  ").c_str(),
      fmr::form::si_unit (this_time, "s").c_str(),
      fmr::form::si_unit (test_count, "reps").c_str());
    mini->data->send (fmr::out, "perf", "Elem", label.c_str(),
      "(%i) %s: %s/%s   AI: %s",
      test_case,
      fmr::form::si_unit (perf.get_busy_flop_speed (), "iops/s").c_str(),
      fmr::form::si_unit (perf.get_flop (), "iops").c_str(),
      fmr::form::si_unit (this_time, "s").c_str(),
      fmr::form::si_unit (double(perf.get_arithmetic_intensity ()), "iops/B").c_str());
    mini->data->send (fmr::out, "perf", "Elem", label.c_str(),
      "(%i) %s: %s/%s chck: %u",
      test_case,
      fmr::form::si_unit (perf.get_busy_data_speed (), "B   /s").c_str(),
      fmr::form::si_unit (perf.get_byte (), "B   ").c_str(),
      fmr::form::si_unit (this_time, "s").c_str(),
      check);
  }

double elem::run_bars_perf_test(const int test_case, const fmr::Local_int test_n) {
  /* These peroforance tests will be used to evaluate different design choices
  * for the Bars element and provide preliminary indications of best practices
  * for Elem variants in general.
  */
  // Start by getting some information to set up the tests.
  //
  // 1 part for each OpenMP thread
  fmr::Local_int  mpi_n = 1, mpi_i = 0, omp_n = 1;
  const auto Pmpi = mini->proc->get_task (Task_type::Fmpi);
  if (Pmpi != nullptr) {
    mpi_n = Pmpi->get_proc_n ();
    mpi_i = Pmpi->get_proc_ix ();
  }
  const auto Pomp = mini->proc->get_task (Task_type::Fomp);
  if (Pomp != nullptr) { omp_n = Pomp->get_proc_n (); }
  const auto part_n = omp_n;
  //
  // Global level variables
  const bool is_periodic = false;
  const fmr::Local_int part_node_n = 1024 * 1024;
  const fmr::Local_int part_elem_n = part_node_n - (is_periodic ? 0 : 1);
  //const fmr::Global_int total_elem_n = part_n * part_elem_n;
  //
  // vector of thread-local performance meters
  std::vector<Perf_time_t*> part_meter(part_n);
  #pragma omp parallel for default(none) \
      firstprivate(part_n) shared(part_meter)
  for (fmr::Local_int part_id = 0; part_id < part_n; ++part_id) {
    // Put a new thread-local meter into part_meter.
    part_meter[part_id] = new Perf_time_t("el");// arg is unit of measure
  }
  // Node coordinates in 1D, 2D, and 3D.
  std::vector<std::valarray<fmr::Geom_float>> part_coor_1d(part_n);
  std::vector<std::valarray<fmr::Geom_float>> part_coor_2d(part_n);
  std::vector<std::valarray<fmr::Geom_float>> part_coor_3d(part_n);
  // Nominal discretization in xyz: [1,1,1] / part_elem_n
  // 1D: bars go along x in a line of length 1
  // 2D: bars go diagonal from bottom left to top right of 1x1 square
  // 3D: bars go diagonal from front bottom left to back top right of 1x1x1 cube
  //
  // Fill the coordinate verctors with thread-local arrays.
  #pragma omp parallel for default(none) \
    shared(part_coor_1d, part_coor_2d, part_coor_3d) \
    firstprivate(part_n, part_elem_n, part_node_n)
  for (fmr::Local_int part_id = 0; part_id < part_n; ++part_id) {
    //
    const fmr::Geom_float sz = 1.0 / part_elem_n;
    const fmr::Geom_float elem_size[3] = {sz, sz, sz};
    //
    fmr::Geom_float part_orig[3] = {// bottom-left node coordinates of this part
      part_id * part_elem_n * elem_size[0],
      part_id * part_elem_n * elem_size[1],
      part_id * part_elem_n * elem_size[2],
    };
    std::valarray<fmr::Geom_float> coor_1d(part_node_n * 1);
    std::valarray<fmr::Geom_float> coor_2d(part_node_n * 2);
    std::valarray<fmr::Geom_float> coor_3d(part_node_n * 3);
    // Fill the coordinate arrays for this part.
    for (fmr::Local_int node_i = 0; node_i < part_node_n; ++node_i) {
      coor_1d[node_i] = node_i * elem_size[0] + part_orig[0];
    }
    part_coor_1d[part_id] = coor_1d;
    for (fmr::Local_int node_i = 0; node_i < part_node_n; ++node_i) {
      coor_2d[2*node_i + 0] = node_i * elem_size[0] + part_orig[0];
      coor_2d[2*node_i + 1] = node_i * elem_size[1] + part_orig[1];
    }
    part_coor_2d[part_id] = coor_2d;
    for (fmr::Local_int node_i = 0; node_i < part_node_n; ++node_i) {
      coor_3d[3*node_i + 0] = node_i * elem_size[0] + part_orig[0];
      coor_3d[3*node_i + 1] = node_i * elem_size[1] + part_orig[1];
      coor_3d[3*node_i + 2] = node_i * elem_size[2] + part_orig[2];
    }
    part_coor_3d[part_id] = coor_3d;
  }
  // Cross section area options: constant and function of position
  // Cross sections evaluated at integration points.

  // vector of thread-local part_conn valarrays
  std::vector<std::valarray<fmr::Local_int>> part_conn_vec(part_n);
  const size_t part_conn_array_size
    = static_cast<size_t>(part_elem_n) * Bars::vert_n;
  // Fill the part_conn_vec with thread-local connectivity arrays.
  #pragma omp parallel for default(none) shared(part_conn_vec) \
    firstprivate(part_n, part_elem_n, part_conn_array_size)
  for (fmr::Local_int part_id = 0; part_id < part_n; ++part_id) {
    const fmr::Local_int grid_size[1] = {part_elem_n * part_n};
    std::valarray<fmr::Local_int> part_conn(part_conn_array_size);
    fmr::Local_int this_conn[2] = {0, 0};
    for (fmr::Local_int elem_i = 0; elem_i < part_elem_n; ++elem_i) {
      femera::grid::elem::get_tile_vert_conn(this_conn, elem_i, grid_size);
      part_conn[2*elem_i + 0] = this_conn[0];
      part_conn[2*elem_i + 1] = this_conn[1];
    }
    part_conn_vec[part_id] = part_conn;
  }
  // First, test non-trivial size structured grids, which can be made by
  // using the Elem tiling methods. At first, we won't no halo exchange.
  // Just measure the performance of the tiling connectivity, comparing 
  // using a traditional connectivity array vs. the integer operations
  // connectivity methods.
  //
  //TODO warm up?
  fmr::Local_int total_sum = 0;
  switch (test_case) {
    case 1:{// Start OpenMP parallel region for the parts,
      for (fmr::Local_int test_i = 0; test_i < test_n; ++test_i) {
        #pragma omp parallel for default(none) \
            firstprivate(part_n, part_elem_n, test_i) shared(part_meter) \
            reduction(+:total_sum)
        for (fmr::Local_int part_id = 0; part_id < part_n; ++part_id) {
          const fmr::Local_int grid_size[1] = {part_elem_n};
          Perf_time_t* this_meter = part_meter[part_id];// thread local ptr
          //
          //Bars this_bars;// Make a Bars instance for this part.
          fmr::Local_int this_conn[2] = {0, 0};

          fmr::Local_int part_chck = 0;
          //
          this_meter->add_idle_time_now ();
          for (fmr::Local_int elem_i = 0; elem_i < part_elem_n; ++elem_i) {
            femera::grid::elem::get_tile_vert_conn(this_conn, elem_i, grid_size);// 4 in
            part_chck += (this_conn[0] + this_conn[1]) % 3;// +1 in,, 1 out, 3 IOPS
            //just so loop does not opitimize away
          }
          this_meter->add_busy_time_now ();
          //
          this_meter->add_unit (part_elem_n);
          this_meter->add_read (5 * sizeof(fmr::Local_int));
          this_meter->add_save (1 * sizeof(fmr::Local_int));
          this_meter->add_flop (3 * part_elem_n);// Actually IOPS, but that's ok.
          total_sum += part_chck;
        }//end parallel for
      }//end test_n loop
    }// end case 1 branch
    break;
    case 2:{
      for (fmr::Local_int test_i = 0; test_i < test_n; ++test_i) {
        #pragma omp parallel for default(none) \
            firstprivate(part_n, part_elem_n, test_i) \
            shared(part_meter, part_conn_vec) \
            reduction(+:total_sum)
        for (fmr::Local_int part_id = 0; part_id < part_n; ++part_id) {
          // Thread local pointers.
          fmr::Local_int* part_conn = &part_conn_vec[part_id][0];
          Perf_time_t* this_meter = part_meter[part_id];
          //
          fmr::Local_int part_chck = 0;
          //
          this_meter->add_idle_time_now ();
          for (fmr::Local_int elem_i = 0; elem_i < part_elem_n; ++elem_i) {
            //just so loop does not optimize away:
            part_chck += (part_conn[2*elem_i + 0] + part_conn[2*elem_i + 1]) % 3;
            // 1+part_conn_array_size in,, 1 out,
            // 3 IOPS (not counting array indexing IOPS)
          }
          this_meter->add_busy_time_now ();
          //
          this_meter->add_unit (part_elem_n);
          this_meter->add_read (
            (1 + part_conn_array_size) * sizeof(fmr::Local_int));
          this_meter->add_save (1 * sizeof(fmr::Local_int));
          this_meter->add_flop (3 * part_elem_n);// Actually IOPS, but that's ok.
          total_sum += part_chck;
        }//end parallel for
      }//end test_n loop

    }// end case 2 branch
    break;
    default:{
      // Handle unknown test cases if necessary
      return 0;
    }
  }// end test_case switch
  //
  double total_test_time = 0.0;
  for (fmr::Local_int i = 0; i < part_n; ++i) {
    total_test_time += double(part_meter[i]->get_busy_s());
  }
  const auto mpi_str
    = "Bars " + std::to_string(mpi_i) + "/" + std::to_string(mpi_n);
  for (fmr::Local_int i = 0; i < part_n; ++i) {// log all from main threads.
    elem::send_perf_results(mpi_str, test_case, *part_meter[i],
      test_n, total_sum / part_n / test_n);
  }
  // total_sum / part_n / test_n == 1048575 for 1024*1024
#if 0
  printf("MPI %u/%u Total test %i time: %es\n", mpi_i, mpi_n, test_case,
    total_test_time);
  printf("MPI %u/%u Average part test %i time: %es\n", mpi_i, mpi_n, test_case,
    total_test_time / part_n / test_n);
  for (fmr::Local_int i = 0; i < part_n; ++i) {
    printf("MPI %u/%u Part %u test %i: %f el\n", mpi_i, mpi_n, i, test_case,
      double(part_meter[i]->get_unit()));
  }
  printf("MPI %u/%u Part test %i total sum: %u\n", mpi_i, mpi_n, test_case,
    total_sum);
#endif
  //return total_sum / part_n / test_n;
  return total_test_time;
}//end run_bars_perf_tests

TEST( GridElemBarsPerf, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
TEST( GridElemBarsPerf, RunBarsPerfTests ){
  const fmr::Local_int test_n = 200; // test count
  EXPECT_LT( \
    elem::run_bars_perf_test(1, test_n), \
    elem::run_bars_perf_test(2, test_n) );
}
} }// end namespace femera::grid::elem
