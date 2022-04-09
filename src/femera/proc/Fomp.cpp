#include "Fomp.hpp"
#include "Main.hpp"
#include "../Data.hpp"

#ifdef FMR_HAS_OPENMP
#include "omp.h"
#endif

#include <unistd.h>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  proc::Fomp::Fomp (const Work::Core_ptrs_t core)
  noexcept : Proc (core) {
    this->name ="OpenMP";
    this->abrv ="omp";
    this->version = std::to_string (_OPENMP);
    this->task_type = task_cast (Plug_type::Fomp);
    //this->proc_ix = this->task_proc_ix ();// set in parallel if >1 Fomp loaded
    this->proc_n  = fmr::Local_int (::omp_get_max_threads ());
  }
#if 0
  void proc::Fomp::task_init (int*, char**) {
    const auto orig_proc_n = this->proc_n;
#else
  void proc::Fomp::task_init (int* argc, char** argv) {
    this->scan (argc, argv);
#endif
#if 0
    if (this->is_in_parallel ()) {//TODO 1 Fomp/team or 1 Fomp/openmp thrd?
      this->proc_n = fmr::Local_int (::omp_get_num_threads ());
    } else {
      if (false && this->proc != nullptr) {//TODO calculate number of OpenMP threads
        this->proc->auto_proc_n ();// sets this->proc_n
        if (this->data != nullptr) {
          const auto name = this->data->text_line ("%4s %4s %4s",
            this->get_base_abrv ().c_str(), this->get_abrv ().c_str(), "thrd");
          data->name_line (data->fmrlog, name.c_str(), "%u", this->proc_n);
      } }
      ::omp_set_num_threads (int (this->proc_n));
      FMR_PRAGMA_OMP(omp parallel) {
        this->proc_n = fmr::Local_int (::omp_get_num_threads ());
  } }
#endif
  }
  void proc::Fomp::scan (int* argc, char** argv) {
    const auto orig_proc_n = this->proc_n;
    if (argc != nullptr && argv != nullptr) {
      FMR_PRAGMA_OMP(omp MAIN) {//NOTE getopt is NOT thread safe.
        int ac=argc[0];// Copy getopt variables.
        auto oe=opterr; auto oo=optopt; auto oi=optind; auto oa=optarg;
        opterr = 0; int optchar;
        while ((optchar = getopt (argc[0], argv, "o:")) != -1) {
          // o:  -o requires an argument//NOTE -g gets eaten by MPI
          //TODO -fmr:o<int> is working because -f -m -r and -: are skipped,
          //     leaving only the recognized option -o<int>.
          switch (optchar) {
            case 'o':{ this->proc_n = fmr::Local_int (atoi (optarg)); break; }
            //TODO: this->proc->opt_add (optchar); break;
        } }
        // Restore getopt variables.
        argc[0]=ac; opterr=oe; optopt=oo; optind=oi; optarg=oa;
    } }
    if (this->proc_n != orig_proc_n) {
      ::omp_set_num_threads (int (this->proc_n));
      FMR_PRAGMA_OMP(omp parallel) {
        this->proc_n = fmr::Local_int (::omp_get_num_threads ());
      }
#ifdef FMR_DEBUG
      printf ("%s Fomp::task_proc_n %i\n", get_abrv ().c_str(),
        ::omp_get_num_threads ());
#endif
      //this->proc->set_base_n ();
#if 0
      if (this->data != nullptr) {
        const auto name = this->data->text_line ("%4s %4s %4s",
          this->get_base_abrv ().c_str(), this->get_abrv ().c_str(), "thrd");
        data->name_line (data->fmrall, name.c_str(), "%4u for process %4u",
          this->proc_n, this->proc->get_proc_id ());//TODO wrong before init done
      }
#endif
  } }
  bool proc::Fomp::is_in_parallel ()
  noexcept {
    return ::omp_in_parallel ();
  }
  fmr::Local_int proc::Fomp::task_proc_ix ()
  noexcept {
#ifdef FMR_DEBUG
    printf ("%s Fomp::task_proc_ix %i\n", get_abrv ().c_str(),
      ::omp_get_thread_num());
#endif
    return fmr::Local_int (::omp_get_thread_num ());
  }
}//end femera:: namespace

#undef FMR_DEBUG
