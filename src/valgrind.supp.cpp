#ifdef FMR_HAS_MPI
#include "mpi.h"
/* This compiles into a minimal executable used to generate a valgrind
 * suppression file that hides expected warnings.
 */
int main(int argc, char **argv) { int err=0;
  int provided=MPI_UNDEFINED;
  err= MPI_Init_thread (&argc,&argv, MPI_THREAD_SERIALIZED,& provided );
  if(!err){
    MPI_Comm comm = MPI_COMM_NULL;
    err= MPI_Comm_dup ( MPI_COMM_WORLD,& comm );
    int proc_id=0; MPI_Comm_rank ( comm,& proc_id );
    double x=0.0;
#ifdef _OPENMP
#pragma omp parallel for reduction(+:x)
#endif
    for(int i=0; i<100; ++i){
      x+=1.0/double(i+proc_id+1);
    }
    double tot=0.0;
    err+= MPI_Reduce (& x,& tot, 1, MPI_DOUBLE, MPI_SUM, 0, comm );
    if(tot<0.0){ MPI_Abort (); }
    MPI_Free (& comm );
    MPI_Finalize ();
    if( provided != MPI_THREAD_SERIALIZED ){ return 1; }
  }
  return err;
}
#else
int main(int, char**){
  double x=0.0;
#ifdef _OPENMP
#pragma omp parallel for reduction(+:x)
#endif
  for(int i=0; i<100; ++i){
    x+=1.0/double(i+1);
  }
  if(x<0.0){ return 1; }
  return 0;
}
#endif
