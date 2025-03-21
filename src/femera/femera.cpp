#include "femera.hpp"
#include "task/Jobs.hpp"

#include <type_traits>

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
  bool fmc::jobs_did_init (fmc::Jobs_t* jobs) {
    return jobs->did_init();
  }
  void fmc::jobs_exit (fmc::Jobs_t* jobs) {
    jobs->exit(0);
  }
  void fmc::delete_jobs(fmc::Jobs_t* jobs) {
    delete jobs;
  }
  const char* fmc::get_version (fmc::Jobs_t* jobs) {
    static std::string jobs_version;
    jobs_version = jobs->get_version();
    return jobs_version.c_str();
  }
  const char* fmc::get_jobs_name (fmc::Jobs_t* jobs) {
    static std::string current_name;
    current_name = jobs->get_name();
    return current_name.c_str();
  }
  const char* fmc::get_sims_name (fmc::Jobs_t* jobs) {
    const auto S = jobs->get_task(femera::Task_type::Sims);
    if (S == nullptr) { return "Could not find any Sims."; } 
    static std::string current_name;
    current_name = S->get_name();
    return current_name.c_str();
  }
  fmr::Dim_int fmc::get_verbosity (fmc::Jobs_t* jobs) {
    if (jobs->data == nullptr) {
      fprintf (stderr, " fmc jobs WARN "
        "Femera data handler not found.\n"
        "Please initialize Femera before calling get_verbosity.\n");
      return 0;
    }
    return jobs->data->get_verb ();
  }
  fmr::Dim_int fmc::set_verbosity (fmc::Jobs_t* jobs, fmr::Dim_int v) {
    if (jobs->data == nullptr) {
      fprintf (stderr, " fmc jobs WARN "
        "Femera data handler not found.\n"
        "Please initialize Femera before calling set_verbosity.\n");
      return 0;
    }
#if 0
    if (std::is_signed<fmr::Dim_int>::value) {
      if (v < 0) {
        jobs->data->send(fmr::log, "jobs","data","NOTE",
          "Negative verbosity is treated as zero (0).");
        v = 0;
    } }
#endif
    if (v > FMR_VERBMAX) {
      jobs->data->send (fmr::log, " fmc","jobs","NOTE",
        "Verbosity clamped to maximum (%i).", FMR_VERBMAX);
      v = FMR_VERBMAX;
    }
    return jobs->data->set_verb (v);
    
  }
}//end extern "C"
