#Uncomment the following line to enable OpenMP
#USE_OPENMP = TRUE
MKLROOT   = /opt/intel/oneapi/mkl/latest/lib/intel64
MKLCUSTOMINTERFACEPATH=/opt/intel/oneapi/mkl/latest/lib/intel64
FFTW_INCL = -I/opt/intel/oneapi/mkl/latest/include/fftw/
FFTW_LIBS   =  ${MKLROOT}/libmkl_scalapack_lp64.a -Wl,--start-group ${MKLROOT}/libmkl_intel_lp64.a ${MKLROOT}/libmkl_sequential.a ${MKLROOT}/libmkl_core.a ${MKLCUSTOMINTERFACEPATH}/libmkl_blacs_custom_lp64.a -Wl,--end-group -lpthread -lm -ldl

DFLAGS = -D__GFORTRAN 
#DFLAGS += -D__IPM

CC = smpicc 
CFLAGS = -O3

FC = smpif90
FFLAGS = -O3 -std=legacy -cpp -x f95-cpp-input #-lg2c

LD = smpif90
LDFLAGS = 

ifeq ($(USE_OPENMP), TRUE)

  #note that this overrides the earlier definition of FFTW_LIBS
  FFTW_LIBS  =  -lfftw3_threads -lfftw3 -lm #-L/opt/fftw/3.3.0.1/x86_64/lib

  DFLAGS    += 

  FFLAGS    += -fopenmp
  LDFLAGS   += -fopenmp

endif

include Makefile.base
