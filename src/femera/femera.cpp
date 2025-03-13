#include "femera.hpp"
#include "task/Jobs.hpp"

fmr::Jobs_t fmr::new_jobs (int* argc, char** argv) {
  FMR_WARN_INLINE_OFF
    return femera::Task<femera::task::Jobs>::new_task (argc,argv);
  FMR_WARN_INLINE_ON
  }
  fmr::Jobs_t fmr::new_jobs () {
  FMR_WARN_INLINE_OFF
    return femera::Task<femera::task::Jobs>::new_task ();
  FMR_WARN_INLINE_ON
  }
extern "C" {
  fmr::Jobs_c* fmr::newc_jobs () {
    return new femera::task::Jobs();
  }
  void fmr::jobs_init (fmr::Jobs_c* jobs) {
    jobs->init(nullptr, nullptr);
  }
  void fmr::jobs_exit (fmr::Jobs_c* jobs) {
    jobs->exit(0);
  }
  void fmr::delete_jobs(fmr::Jobs_c* jobs) {
    delete jobs;
  }
}
