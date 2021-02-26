#include "../Perf.hpp"

#include <cstdio>  // std::printf

int main(){
  using namespace Femera;
  //NOTE The use of true and false conditionals is intentional, to duplicate
  //     the usual use of compiler pre-processing variables.
  //
  const int iters = 100000; float x=0.4;
  Perf::Timer timer_timer;
  //
  // Get everything loaded for use below.
  perf_timer_start (& timer_timer );
  perf_timer_resume(& timer_timer );
  x *= 0.99;// trivial operation that will not be optimized away and used below.
  perf_timer_pause (& timer_timer );
  perf_timer_stop  (& timer_timer );
  //
  // First, iterate without inner loop timing to obtain a reference time.
  // Do this when all the time spent in an inner loop should count as work.
  perf_timer_start(& timer_timer );// Start paused, counting idle time.
  if( true ){    perf_timer_resume(& timer_timer ); }
  for( int i=0; i<iters; i++){
    if( false ){ perf_timer_resume(& timer_timer ); }
    x *= 0.99;
    if( false ){ perf_timer_pause (& timer_timer ); }
  }
  if( true ){    perf_timer_pause (& timer_timer, iters ); }
  perf_timer_stop(& timer_timer );// Capture wind-down time.
  //
  float trivial_loop_ns    = perf_timer_work_ns( timer_timer );
  float trivial_loop_speed = perf_work_speed   ( timer_timer );
  //
  // Then, iterate with inner loop timing.
  perf_timer_start(& timer_timer );// Start paused, counting non-work time.
  if( false ){  perf_timer_resume(& timer_timer ); }
  for( int i=0; i<iters; i++){
    if( true ){ perf_timer_resume(& timer_timer ); }
    x *= 0.99;
    if( true ){ perf_timer_pause (& timer_timer ); }
    // There is no non-work time, because this is timing the timer itself.
  }
  if( false ){  perf_timer_pause (& timer_timer, iters ); }
  perf_timer_stop(& timer_timer );// Capture wind-down time.
  //
  float timed_inner_ns   = perf_timer_work_ns( timer_timer );
  float timed_loop_speed = perf_work_speed   ( timer_timer );
  //
  float inner_overhead_ns = timed_inner_ns - trivial_loop_ns;
  //
  std::printf( "%8.1f  ns/iter  trivial loop time\n",
    trivial_loop_ns / iters );
  std::printf( "%8.1f  ns/iter  inner loop timer overhead\n",
    inner_overhead_ns / iters );
  std::printf( "%8.1f  Miter/s  trivial loop speed limit\n", 1e-6*
    trivial_loop_speed );
  std::printf( "%8.1f  Miter/s  timed inner loop speed limit\n", 1e-6*
    timed_loop_speed );
  //
  return 0 + int(x);// Use x so the trivial loop is not optimized away.
}
