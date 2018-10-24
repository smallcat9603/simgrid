/* -*- Mode: C; -*- */
/* Creator: Bronis R. de Supinski (bronis@llnl.gov) Fri Mar  17 2000 */
/* no-error.c -- do some MPI calls without any errors */

#include <stdio.h>
#include "mpi.h"

#define buf_size 128

#define TWOD     2

int
main (int argc, char **argv)
{
  int nprocs = -1;
  int rank = -1;
  char processor_name[128];
  int namelen = 128;
  int buf0[buf_size];
  int buf1[buf_size];
  MPI_Status status;
  MPI_Comm temp, comm;
  MPI_Group world_group, dgroup;
  int *granks;
  int dims[TWOD], periods[TWOD], remain_dims[TWOD];
  int drank, dnprocs;

  /* init */
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name (processor_name, &namelen);
  printf ("(%d) is alive on %s\n", rank, processor_name);
  fflush (stdout);

  MPI_Barrier (MPI_COMM_WORLD);

  if (nprocs < 4) {
    printf ("not enough tasks\n");
  }
  else {
    /* need to make cartesian communicator temporarily... */
    /* create a 2 X nprocs/2 torus topology, allow reordering */
    dims[0] = 2;
    dims[1] = nprocs/2;
    periods[0] = periods[1] = 1;
    MPI_Cart_create (MPI_COMM_WORLD, TWOD, dims, periods, 1, &temp);

    if (temp != MPI_COMM_NULL) {
      /* create 2 1 X nprocs/2 topologies... */
      remain_dims[0] = 0;
      remain_dims[1] = 1;
      MPI_Cart_sub (temp, remain_dims, &comm);
      /* free up temporarily created cartesian communicator... */
      MPI_Comm_free (&temp);
    }
    else {
      comm = MPI_COMM_NULL;
    }

    if (comm != MPI_COMM_NULL) {
      MPI_Comm_size (comm, &dnprocs);
      MPI_Comm_rank (comm, &drank);

      if (dnprocs > 1) {
	if (drank == 0) {
	  memset (buf0, 0, buf_size*sizeof(int));

	  MPI_Recv (buf1, buf_size, MPI_INT, 1, 0, comm, &status);
	
	  MPI_Send (buf0, buf_size, MPI_INT, 1, 0, comm);
	}
	else if (drank == 1) {
	  memset (buf1, 1, buf_size*sizeof(int));

	  MPI_Recv (buf0, buf_size, MPI_INT, 0, 0, comm, &status);

	  MPI_Send (buf1, buf_size, MPI_INT, 0, 0, comm);
	}
      }
      else {
	printf ("(%d) Derived communicator too small (size = %d)\n",
		rank, dnprocs);
      }

      MPI_Comm_free (&comm);
    }
    else {
      printf ("(%d) Got MPI_COMM_NULL\n", rank);
    }
  }

  MPI_Barrier (MPI_COMM_WORLD);

  MPI_Finalize ();
  printf ("(%d) Finished normally\n", rank);
}

/* EOF */
