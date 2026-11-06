#ifndef FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP
#define FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP

#include "../Elem.hpp"

#include "../../../fmr/perf/Meter.hpp"

namespace femera { namespace grid { namespace elem {//TODO move to test namespace?

void send_perf_results (std::string label, int test_case,
  fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>& perf,
  uint test_count, uint check);

double run_bars_perf_test(const int test_case,
  const fmr::Local_int test_n=1);

} } }//end namespace femera::grid::elem

//end FEMERA_HAS_GRID_ELEM_BARS_PERF_HPP
#endif
