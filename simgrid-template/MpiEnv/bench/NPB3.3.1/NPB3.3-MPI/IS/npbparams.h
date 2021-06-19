#define CLASS 'A'
#define NUM_PROCS 64
/*
   This file is generated automatically by the setparams utility.
   It sets the number of processors and the class of the NPB
   in this directory. Do not modify it by hand.   */
   
#define COMPILETIME "21 Jun 2017"
#define NPBVERSION "3.3.1"
#define MPICC "${SIMGRID_PATH}/bin/smpicc"
#define CFLAGS "-O"
#define CLINK "$(MPICC)"
#define CLINKFLAGS "-O -L${SIMGRID_PATH}/lib"
#define CMPI_LIB "-lsimgrid -lgfortran #-lsmpi -lgras  "
#define CMPI_INC "-I${SIMGRID_PATH}/include/smpi/"
