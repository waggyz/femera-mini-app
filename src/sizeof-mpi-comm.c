#include <mpi.h>
#include <stdio.h>

int main (void) {
  printf ("%lu\n", sizeof (MPI_Comm) );
  return 0;
}

