#include "../Perf.hpp"

#include <cstdio>  // std::printf
#include <unistd.h> // usleep

#define ENABLE_TIMING_DETAIL

#ifdef ENABLE_TIMING_DETAIL
#define THIS_TIMING_DETAIL true
#else
#define THIS_TIMING_DETAIL false
#endif
int main(){
  using namespace Femera;
  const int iters=10, nap_us = 1000;
  Perf::Timer sleep_time;
  //
  // Start paused, counting idle time.
  perf_timer_start(& sleep_time );
  std::printf("Getting sleepy...\n");
  if( ! THIS_TIMING_DETAIL ){ perf_timer_resume(& sleep_time ); }
  for( int i=0; i<iters; i++){
    // Unpause timer right before...
    if( THIS_TIMING_DETAIL ){ perf_timer_resume(& sleep_time ); }
    usleep( nap_us );
    if( THIS_TIMING_DETAIL ){ perf_timer_pause(& sleep_time );
    //...pause immediately after.
      if( i == 0 ){
        std::printf(
          "    nap %2i : %9li awake %9li asleep %9li ns accumulated sleep\n",
          sleep_time.count, sleep_time.last_idle_ns, sleep_time.last_work_ns,
          sleep_time.work_ns ); }
      else{
        std::printf(
          "    nap %2i : %9li       %9li        %9li ns\n",
          sleep_time.count, sleep_time.last_idle_ns, sleep_time.last_work_ns,
          sleep_time.work_ns ); }
  } }
  if( ! THIS_TIMING_DETAIL ){ perf_timer_pause(& sleep_time, iters ); }
  std::printf("...done %i naps (1 nap = %i microsleeps).\n",
    sleep_time.count, nap_us );
  std::printf(" Nap speed : %.0f  naps/s\n",
    perf_work_speed( sleep_time ) );
  perf_timer_stop(& sleep_time );// Capture wind-down time.
  //
  // Work is done. Now summarize the results.
  const float ns=1e-9, pct=100.0;
  const float  time_theor = float( iters ) * float( nap_us ) *1e-6 ;
  const float speed_theor = float( iters ) / float( nap_us ) *1e+6 ;
  const float  meas_speed = perf_overall_speed( sleep_time );
  std::printf("   Overall : %.0f  naps/s\n", meas_speed );
  std::printf("  Overhead : %5.1f%%\n", pct*
    perf_timer_idle_ns( sleep_time ) / perf_timer_total_ns( sleep_time ) );
  std::printf("Efficiency : %5.1f%% of theoretical nap speed.\n", pct*
    (1.0 - meas_speed / speed_theor ) );
  std::printf(" Overslept : %5.1fx\n",
    float( perf_timer_total_ns( sleep_time ) ) *ns / time_theor );
  //
  return 0;
}
#undef THIS_TIMING_DETAIL
