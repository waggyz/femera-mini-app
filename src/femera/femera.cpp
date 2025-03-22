#include "femera.hpp"
#include "task/Jobs.hpp"
#include "task/Sims.hpp"

#include <type_traits>

// Jobs ==================================================================
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
  fmc::Jobs_t* fmc::fmr_new_jobs () {
    return new femera::task::Jobs();
  }
  void fmc::fmr_jobs_init (fmc::Jobs_t* jobs) {
    jobs->init(nullptr, nullptr);
  }
  bool fmc::fmr_jobs_did_init (fmc::Jobs_t* jobs) {
    return jobs->did_init();
  }
  void fmc::fmr_jobs_exit (fmc::Jobs_t* jobs) {
    jobs->exit(0);
  }
  void fmc::fmr_delete_jobs(fmc::Jobs_t* jobs) {
    delete jobs;
  }
  const char* fmc::fmr_get_version (fmc::Jobs_t* jobs) {
    static std::string jobs_version;
    jobs_version = jobs->get_version();
    return jobs_version.c_str();
  }
  const char* fmc::fmr_get_jobs_name (fmc::Jobs_t* jobs) {
    static std::string current_name;
    current_name = jobs->get_name();
    return current_name.c_str();
  }
  fmr::Dim_int fmc::fmr_get_verbosity (fmc::Jobs_t* jobs) {
    if (jobs->data == nullptr) {
      fprintf (stderr, " fmc jobs WARN "
        "Femera data handler not found.\n"
        "Please initialize Femera before calling get_verbosity.\n");
      return 0;
    }
    return jobs->data->get_verb ();
  }
  fmr::Dim_int fmc::fmr_set_verbosity (fmc::Jobs_t* jobs, fmr::Dim_int v) {
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
/*  fmc::Sims_t* fmc::fmr_add_sims (fmc::Jobs_t* jobs) {
    const auto i = jobs->add_task (std::move
      (femera::Task<femera::task::Sims>::new_task (jobs->get_core())));
    if (false) {
      //TODO see if storage is needed.
      auto S = jobs->get_task(femera::Task_type::Sims, i);
      static fmc::Sims_t* stored_ptr;// Store it to extend its lifetime.
      stored_ptr = reinterpret_cast<fmc::Sims_t*>(S);
      return stored_ptr;
    } else {
      // use if storage is not needed
      return reinterpret_cast<fmc::Sims_t*>//TODO try a less dangerous cast.
        (jobs->get_task(femera::Task_type::Sims, i));
    }*/
  fmr::Local_int fmc::fmr_add_sims (fmc::Jobs_t* jobs) {
    return jobs->add_task (std::move
      (femera::Task<femera::task::Sims>::new_task (jobs->get_core())));
  }
  // Sims operations ---------------------------------------------------------
  const char* fmc::fmr_get_sims_name (fmc::Jobs_t* jobs) {
    const auto S = jobs->get_task (femera::Task_type::Sims,
      jobs->get_task_n(femera::Task_type::Sims) -1);
    static std::string current_name;
    if (S == nullptr) {
      current_name = "No Sims found in fmr_get_sims_name (Jobs*).";
    } else {
      current_name = S->get_name ();
    }
    return current_name.c_str ();
  }
  void fmc::fmr_set_sims_name (fmc::Jobs_t* jobs, const char* name) {
    const auto S = jobs->get_task (femera::Task_type::Sims,
      jobs->get_task_n(femera::Task_type::Sims) -1);
    if (S == nullptr) { return; }
    S->set_name (name);
    return;
  }
  const char* fmc::fmr_get_sims_version (fmc::Jobs_t* jobs) {
    const auto S = jobs->get_task (femera::Task_type::Sims,
      jobs->get_task_n(femera::Task_type::Sims) -1);
    if (S == nullptr) { return " fmc jobs WARN Could not find any Sims."; }
    static std::string current_vers;
    current_vers = S->get_version ();
    return current_vers.c_str ();
  }
  void fmc::fmr_set_sims_version (fmc::Jobs_t* jobs, const char* name) {
    const auto S = jobs->get_task (femera::Task_type::Sims,
      jobs->get_task_n(femera::Task_type::Sims) -1);
    if (S == nullptr) { return; }
    S->set_version (name);
    return;
  }
  // ==========================================================================
}//end extern "C"
