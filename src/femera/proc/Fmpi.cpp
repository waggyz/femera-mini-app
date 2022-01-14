#include "Fmpi.hpp"

#include <exception>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

#ifdef FMR_HAS_MPI
#include "mpi.h"
namespace femera {
  //
  proc::Fmpi::Fmpi (femera::Work::Core_t W)
  noexcept
  : mpi_required {int (MPI_THREAD_SERIALIZED) } {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="MPI";
    this->info_d = 3;
    this->team_id = proc::Team_t (MPI_COMM_WORLD);
    this-> version =
      std::to_string( MPI_VERSION )
      +std::string(".")
      +std::to_string( MPI_SUBVERSION );
  }
  bool proc::Fmpi::is_mpi_init () {
    int is_init=0;
    int err = MPI_Initialized (& is_init);
    //FIXME Be careful handling errors to shut down MPI properly before exit.
    //if (err) { FMR_THROW("proc::Fmpi::is_mpi_init() failed"); }
    return bool (err) | bool (is_init);
  }
  fmr::Local_int proc::Fmpi::task_proc_n () {int err=0, tmp_proc_n=0;
    err= MPI_Comm_size(MPI_Comm(this->team_id) ,& tmp_proc_n  );
    if( err ){return 1; }
    this->proc_n = fmr::Local_int (tmp_proc_n);
    return this->proc_n;
  }
  fmr::Local_int proc::Fmpi::task_proc_ix () {int err, proc_i=0;
    err= MPI_Comm_rank( MPI_Comm(this->team_id),& proc_i );
    if( err ){return 0; }
    this->proc_ix = fmr::Local_int (proc_i);
    return this->proc_ix;
  }
  void proc::Fmpi::task_init (int* argc, char** argv){ int err=0;
//  fmr::perf:: timer_resume (&this->time);
  if (sizeof(this->team_id) != sizeof(MPI_Comm)){
    std::fprintf (stderr,
      "WARNING sizeof (Proc::team_id) is %lu but should be %lu\n",
      sizeof(this->team_id), sizeof(MPI_Comm) );
  }
  if (sizeof(this->mpi_required) != sizeof(MPI_THREAD_SERIALIZED)){
    std::fprintf (stderr,
      "WARNING sizeof (Proc::mpi_required) is %lu but should be %lu\n",
      sizeof(this->mpi_required), sizeof(MPI_Comm) );
  }
  if (is_mpi_init ()) {
    this->do_final_on_exit = false;
    std::printf ("MPI is already initialized.\n");
  }
  if (!err && !is_mpi_init ()) {
    err= MPI_Init_thread( argc,&argv, this->mpi_required, &this->mpi_provided);
#ifdef FMR_DEBUG
    std::printf("%u:Fmpi::task_init (%i) start...\n", this->get_proc_i(),err);
#endif
  }
  if (is_mpi_init ()) {
    MPI_Comm comm =nullptr;
    if( !err ){// Good practice: use a copy of MPI_COMM_WORLD.
      err= MPI_Comm_dup (MPI_COMM_WORLD, &comm);// exit_task() frees this
    }
    if( !err ){
      this->team_id = proc::Team_t (comm);
      this->proc_ix = this->task_proc_ix ();
      this->proc_n  = this->task_proc_n ();
    }
//  fmr::perf:: timer_pause (&this->time);
  } }
  void proc::Fmpi::task_exit () {int err=0;                 //FIXME not an arg
#ifdef FMR_DEBUG
    std::printf("Fmpi::task_exit (%i) start...\n", err);
#endif
//  fmr::perf::timer_resume (&this->time);
//    this->barrier ();//TODO NEEDED?
#if 0
  if( err>0 ){ log-> printf_err("ERROR Femera returned %i\n",err); }
#else
  // OpenMP is already deleted, so log->printf(..) may no longer work?
  if (err>0) {std::fprintf (stderr, "ERROR Femera returned %i\n", err);}
#endif
  err=0;// Exit from mpi normally when Femera exits on error.
FMR_PRAGMA_OMP(omp master)
  if (is_mpi_init ()) {
    const auto proc_i = this->task_proc_ix ();
    if (this->team_id != proc::Team_t (MPI_COMM_WORLD) ) {
      if (this->team_id) {
#ifdef FMR_DEBUG
        std::printf("%u:Fmpi::task exit MPI_Comm_free (%lu)...\n",
        proc_i, team_id);
#endif
        MPI_Comm f=MPI_Comm (this->team_id);
        err= MPI_Comm_free (& f);
        if (err>0) {
          std::fprintf (stderr,
            "ERROR MPI_Comm_free(%lu) in process %u returned %i\n",
            this->team_id, proc_i, err);
        }
#ifdef FMR_DEBUG
        std::printf("%u:Fmpi::task exit MPI_Comm_free (%lu)\n",
          proc_i, proc::Team_t (f));
#endif
      }
      this->team_id = proc::Team_t (MPI_COMM_WORLD);//TODO Set to nullptr?
#ifdef FMR_DEBUG
        std::printf("%u:Fmpi::team_id reset to MPI_COMM_WORLD (%lu)\n",
        proc_i, team_id);
#endif
    }
    if (this->do_final_on_exit) {
#ifdef FMR_DEBUG
      std::printf("%u:Fmpi::exit_task MPI_finalize ()...\n", proc_i);
#endif
      err= MPI_Finalize();
      if (err>0) {//log-> printf_err
//        ("ERROR MPI_Finalize() returned %i\n",err);
        std::fprintf (stderr, "ERROR MPI_Finalize() returned %i\n", err);
  } } }
//  fmr::perf:: timer_pause (&this->time);
#ifdef FMR_DEBUG
  std::printf("Fmpi::exit_task (%i) done...\n", err);
#endif
  }
  //
}// end femera:: namespace
//end if FMR_HAS_MPI
#endif

#undef FMR_DEBUG
