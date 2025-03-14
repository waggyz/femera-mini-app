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
  fmc::Jobs_t* fmc::new_jobs () {
    return new femera::task::Jobs();
  }
  void fmc::jobs_init (fmc::Jobs_t* jobs) {
    jobs->init(nullptr, nullptr);
  }
  void fmc::jobs_exit (fmc::Jobs_t* jobs) {
    jobs->exit(0);
  }
  void fmc::delete_jobs(fmc::Jobs_t* jobs) {
    delete jobs;
  }
  const char* fmc::get_version (fmc::Jobs_t* jobs) {
    static std::string version = jobs->get_version();
    return version.c_str();
  }
}
