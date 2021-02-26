#include "gtest/gtest.h"
#include "../Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
  Gtst::Gtst (Proc* P,Data* D){this->proc=P; this->data=D; this->log=proc->log;
    this-> work_type = work_cast(Plug_type::Gtst);
//    this-> base_type = work_cast (Base_type::Proc);
    this-> task_name ="GoogleTest";
    this-> verblevel = 2;
    this-> hier_lv   =-1;// not in processing hierarchy
  }
  int Gtst::prep (){
    this->proc_n=1;
    const char* ver =// not easy to get googletest version
#include "build-data/googletest-version.inc"
    ;
    this-> version = std::string( ver );
    return 0;
  }
  int Gtst::chck (){int err=0;
    fmr::perf::timer_resume (&this->time);
    if (this->is_enabled && !ran_all_tests){
      this->ran_all_tests = true;
      err= RUN_ALL_TESTS();//NOTE macro runs regardless of being enabled
      if (err){
        if (this->proc->is_master()){
          this->proc->log->fprintf (stderr,
            "WARN""ING GoogleTest returned %i\n",err);
      } } }
    fmr::perf::timer_pause (&this->time);
    return err;
  }
  int Gtst::init_task (int* argc, char** argv){ int err=0;
    fmr::perf::timer_resume (&this->time);
#ifdef FMR_DEBUG
    std::printf("Gtst::init_task()...\n");
#endif
    err = this->prep ();
    FMR_PRAGMA_OMP(omp master) {//NOTE getopt is NOT thread safe.
      int argc2=argc[0];// Copy getopt variables.
      auto opterr2=opterr; auto optopt2=optopt;
      auto optind2=optind; auto optarg2=optarg;
      opterr = 0; int optchar;
      while ((optchar = getopt (argc[0], argv, "T")) != -1){
        // T  -T has no argument//NOTE -g is eaten by MPI
        switch (optchar) {
          case 'T':{ this->is_enabled=true; this->proc->opt_add ('T'); break; }
      } }
      // Restore getopt variables.
      argc[0]=argc2;opterr=opterr2;optopt=optopt2;optind=optind2;optarg=optarg2;
    }
    // init even if not is_enabled as it could be enabled later.
    ::testing::InitGoogleTest (argc, argv);//TODO is this thread safe?
#ifdef FMR_HAS_MPI
    //from: https://github.com/google/googletest/issues/822
    ::testing::TestEventListeners& listeners
      = ::testing::UnitTest::GetInstance()->listeners();
    if (!this->is_enabled){
      delete listeners.Release (listeners.default_result_printer());
    }else{
    if (!this->proc->is_master()){
      delete listeners.Release (listeners.default_result_printer());
    } }
#endif
    fmr::perf::timer_pause (&this->time);
    return err;
  }
  int Gtst::exit_task (int err){
#ifdef FMR_DEBUG
    std::printf("Gtst::exit(%i)...\n",err);
#endif
    if (!err){this->chck(); }
    return err;
  }
}// end Femera namespace
#undef FMR_DEBUG
