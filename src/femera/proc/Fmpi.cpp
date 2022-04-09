#include "Fmpi.hpp"

#include <exception>
#include <valarray>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

#ifdef FMR_HAS_MPI
#include "mpi.h"
namespace femera {
  //
  proc::Fmpi::Fmpi (const femera::Work::Core_ptrs_t W)
  noexcept : Proc (W), fmpi_required {int (MPI_THREAD_SERIALIZED) } {
    this->name ="MPI";
    this->abrv ="mpi";
    this->version = std::to_string (MPI_VERSION)
      +"."+std::to_string (MPI_SUBVERSION);
    this->task_type = task_cast (Plug_type::Fmpi);
    this->info_d = 3;
    this->team_id = proc::Team_t (MPI_COMM_WORLD);
  }
  bool proc::Fmpi::did_mpi_init ()
  noexcept {
    int is_init=0;
    int err = MPI_Initialized (& is_init);
    //NOTE Be careful handling errors to shut down MPI properly before exit.
    //if (err) { FMR_THROW("proc::Fmpi::did_mpi_init() failed"); }
    return (err==0) & bool (is_init);
  }
  fmr::Local_int proc::Fmpi::task_proc_n ()
  noexcept {int err=0, n=0;
    err= MPI_Comm_size (MPI_Comm (this->team_id), & n);
    if (err) { return 1; }
    this->proc_n = fmr::Local_int (n);
    return this->proc_n;
  }
  fmr::Local_int proc::Fmpi::task_proc_ix ()
  noexcept {int err =0;
    if (this->did_mpi_init ()) {
      int proc_i =0;
      err= MPI_Comm_rank (MPI_Comm (this->team_id), & proc_i);
      if (err) { return 0; }
      this->proc_ix = fmr::Local_int (proc_i);
    }
    return this->proc_ix;
  }
  void proc::Fmpi::task_init (int* argc, char** argv) {int err=0;
    std::valarray<char> buf (MPI_MAX_LIBRARY_VERSION_STRING);
    // MPI_MAX_LIBRARY_VERSION_STRING includes space for the terminating null.
    int buflen=0, mpiver=0, mpisub=0;
    err= MPI_Get_library_version (& buf[0], & buflen);
    err= MPI_Get_version (& mpiver, & mpisub);
    this->set_name ("MPI "+std::to_string (mpiver)+"."+std::to_string (mpisub)
      +" ("+std::string (& buf[0])+")");
    if (sizeof (this->team_id) != sizeof (MPI_Comm)) {
      std::fprintf (::stderr,
        "WARNING sizeof (Proc::team_id) is %lu but should be %lu\n",
        sizeof (this->team_id), sizeof (MPI_Comm));
    }
    if (sizeof (this->fmpi_required) != sizeof (MPI_THREAD_SERIALIZED)) {
      std::fprintf (::stderr,
        "WARNING sizeof (Proc::mpi_required) is %lu but should be %lu\n",
        sizeof (this->fmpi_required), sizeof (MPI_Comm));
    }
    if (did_mpi_init ()) {
      this->do_final_on_exit = false;
      std::printf ("MPI is already initialized.\n");
    }
    if (!err && !did_mpi_init ()) {
      err= MPI_Init_thread (argc,&argv, this->fmpi_required, & fmpi_provided);
#ifdef FMR_DEBUG
      std::printf ("%u:Fmpi::task_init (%i) start...\n", get_proc_ix(), err);
#endif
    }
    if (did_mpi_init ()) {
      MPI_Comm comm =nullptr;
      if (!err) {//NOTE Good practice: use a copy of MPI_COMM_WORLD.
        err= MPI_Comm_dup (MPI_COMM_WORLD, & comm);// task_exit() frees this
      }
      if (!err) {
        this->team_id = proc::Team_t (comm);
        this->proc_ix = this->task_proc_ix ();
        this->proc_n  = this->task_proc_n ();
#ifdef FMR_DEBUG
      std::printf ("Fmpi::task_init: %u/%u processes...\n", proc_ix, proc_n);
#endif
  } } }
  void proc::Fmpi::task_exit () {int err=0;
#ifdef FMR_DEBUG
    std::printf ("Fmpi::task_exit (%i) start...\n", err);
#endif
  // data handler has already exited
  if (err) { std::fprintf (::stderr, "%s Femera returned %i\n",
    (err<0) ?"WARNING":"ERROR", err); }
  err=0;// Exit from MPI normally when Femera exits on error.
  FMR_PRAGMA_OMP(omp MAIN)
  if (did_mpi_init ()) {
    const auto proc_i = this->task_proc_ix ();
    if (this->team_id != proc::Team_t (MPI_COMM_WORLD)) {
      if (this->team_id) {
#ifdef FMR_DEBUG
        std::printf ("%u:Fmpi::task_exit MPI_Comm_free (%lu)...\n",
          proc_i, team_id);
#endif
        MPI_Comm comm = MPI_Comm (this->team_id);
        err= MPI_Comm_free (& comm);
        if (err) {
          std::fprintf (::stderr,
            "ERROR MPI_Comm_free(%lu) in process %u returned %i\n",
            this->team_id, proc_i, err);
        }
#ifdef FMR_DEBUG
        std::printf ("%u:Fmpi::task_exit MPI_Comm_free (%lu)\n",
          proc_i, proc::Team_t (comm));
#endif
      }
      this->team_id = proc::Team_t (MPI_COMM_WORLD);
#ifdef FMR_DEBUG
        std::printf ("%u:Fmpi::team_id reset to MPI_COMM_WORLD (%lu)\n",
          proc_i, team_id);
#endif
    }
    if (this->do_final_on_exit) {
#ifdef FMR_DEBUG
      std::printf ("%u:Fmpi::exit_task MPI_finalize ()...\n", proc_i);
#endif
      err= MPI_Finalize ();
      if (err) {
        std::fprintf (::stderr, "ERROR MPI_Finalize() returned %i\n", err);
  } } }
#ifdef FMR_DEBUG
  std::printf ("Fmpi::exit_task (%i) done...\n", err);
#endif
  }
  //
}// end femera:: namespace
//end if FMR_HAS_MPI
#endif

#undef FMR_DEBUG
