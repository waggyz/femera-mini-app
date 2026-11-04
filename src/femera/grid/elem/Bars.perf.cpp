#include "Bars.perf.hpp"
#include "../../femera.hpp"
#include "Bars.hpp"

#include "../../../fmr/perf/Meter.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}
using Perf_time_t = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>;

namespace femera { namespace grid {

fmr::Local_int elem::run_bars_perf_tests(const int test_case, const fmr::Local_int test_n) {
  /* These peroforance tests will be used to evaluate different design choices
  * for the Bars element and provide preliminary indications of best practices
  * for Elem variants in general..
  * 
  */


  // Start by getting some information to set up the tests.
  //
  // 1 part for each OpenMP thread
  fmr::Local_int omp_n = 1;
  const auto Pomp = mini.proc->get_task (Task_type::Fomp);
  if (Pomp != nullptr) { omp_n = Pomp->get_proc_n (); }
  const auto part_n = omp_n;
  //
  // Global level variables
  const fmr::Local_int part_elem_n = 1024 * 1024;
  //const fmr::Global_int total_elem_n = part_n * part_elem_n;

  // Vector of placeholders for performance meters
  std::vector<Perf_time_t*> part_meter(part_n);
  //
  // First, test non-trivial size structured grids, which can be made by
  // using the Elem tiling methods. At first, we won't no halo exchange.
  // Just measure the performance of the tiling connectivity, comparing 
  // using a traditional connectivity array vs. the integer operations
  // connectivity methods.
  //
  //TODO warm up?
  // Start OpenMP parallel region for the parts,
  // with thread-local copies of part_n and part_elem_n
  fmr::Local_int total_sum = 0;
  switch (test_case) {
    case 1:{
      for (fmr::Local_int test_i = 0; test_i < test_n; ++test_i) {
        #pragma omp parallel for default(none) \
            firstprivate(part_n, part_elem_n, test_i) shared(part_meter) \
            reduction(+:total_sum)
        for (fmr::Local_int part_id = 0; part_id < part_n; ++part_id) {
          const fmr::Local_int grid_size[1] = {part_elem_n};
          if (test_i == 0){// Put a new thread-local meter into part_meter.
            part_meter[part_id] = new Perf_time_t("el");// arg is unit of measure
          }
          Perf_time_t* this_meter = part_meter[part_id];// entirely thread local ref.
          // Make a Bars instance for this part.
          //Bars this_bars;
          fmr::Local_int this_elem[2] = {0, 0};

          fmr::Local_int part_sum = 0;
          //
          this_meter->add_idle_time_now ();
          for (fmr::Local_int elem_i = 0; elem_i < part_elem_n; ++elem_i) {
            //this_bars.get_tile_conn(this_elem, elem_i, grid_size);
            femera::grid::elem::get_tile_conn(this_elem, elem_i, grid_size);// 4 in
            part_sum += (this_elem[0] + this_elem[1]) % 3;// +1 in,, 1 out, 3 IOPS
            //just so loop does not opitimize away
          }
          this_meter->add_busy_time_now ();
          //
          this_meter->add_unit (part_elem_n);
          this_meter->add_read (5 * sizeof(fmr::Local_int));
          this_meter->add_save (1 * sizeof(fmr::Local_int));
          this_meter->add_flop (3 * part_elem_n);// Actually IOPS, but that's ok.
          total_sum += part_sum;
        }//end parallel for
      }//end test_n loop
    }// end case 1 branch
    break;
    case 2:{
      for (fmr::Local_int test_i = 0; test_i < test_n; ++test_i) {
        #pragma omp parallel for default(none) \
            firstprivate(part_n, part_elem_n, test_i) shared(part_meter) \
            reduction(+:total_sum)
        for (fmr::Local_int part_id = 0; part_id < part_n; ++part_id) {
          const fmr::Local_int grid_size[1] = {part_elem_n};
          const size_t part_conn_array_size
            = static_cast<size_t>(part_elem_n) * Bars::vert_n;
          std::valarray<fmr::Local_int> part_conn(part_conn_array_size);
          fmr::Local_int this_elem[2] = {0, 0};
          for (fmr::Local_int elem_i = 0; elem_i < part_elem_n; ++elem_i) {
            femera::grid::elem::get_tile_conn(this_elem, elem_i, grid_size);
            part_conn[2*elem_i + 0] = this_elem[0];
            part_conn[2*elem_i + 1] = this_elem[1];
          }
          if (test_i == 0){// Put a new thread-local meter into part_meter.
            part_meter[part_id] = new Perf_time_t("el");// arg is unit of measure
          }
          Perf_time_t* this_meter = part_meter[part_id];// entirely thread local ref.
          fmr::Local_int part_sum = 0;
          //
          this_meter->add_idle_time_now ();
          for (fmr::Local_int elem_i = 0; elem_i < part_elem_n; ++elem_i) {
            //just so loop does not optimize away:
            part_sum += (part_conn[2*elem_i + 0] + part_conn[2*elem_i + 1]) % 3;
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
          total_sum += part_sum;
        }//end parallel for
      }//end test_n loop

    }// end case 2 branch
    break;
    default:{
      // Handle unknown test cases if necessary
    }
  }// end test_case switch

  double total_test_time = 0.0;
  for (fmr::Local_int i = 0; i < part_n; ++i) {
    total_test_time += double(part_meter[i]->get_busy_s());
  }
  printf("Total test %i time: %es\n", test_case, total_test_time);
  printf("Average part test %i time: %es\n", test_case,
    total_test_time / part_n / test_n);
  for (fmr::Local_int i = 0; i < part_n; ++i) {
    printf("Part %u test %i: %f el\n", i, test_case,
      double(part_meter[i]->get_unit()));
  }
  printf("Part test %i total sum: %u\n", test_case, total_sum);
  return total_sum / part_n / test_n;
}//end run_bars_perf_tests

TEST( GridElemBarsPerf, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
TEST( GridElemBarsPerf, RunBarsPerfTests ){
  const fmr::Local_int test_n = 200; // test count
  EXPECT_EQ( elem::run_bars_perf_tests(1, test_n), 1048575 );// 1048575 for 1024*1024
  EXPECT_EQ( elem::run_bars_perf_tests(2, test_n), 1048575 );
}
} }// end namespace femera::grid::elem
