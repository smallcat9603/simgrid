export USE_OPENMP=1
# export USE_LIBXC=1
# define HAS_BACKTRACE if glibc backtrace functionality is available
# export HAS_BACKTRACE=1

export LIBS += 
export ADD_LIBS += -L/usr/local/lib $(HDF) -lblas -llapack \
                   -lgfortran

export INC_PATH += -I/usr/include/hdf5/serial/
export CFLAGS=-O3 -g -fPIC -DADD_
export MYCFLAGS=-O3 -g -fPIC -DADD_
export FFLAGS=-O3 -g -fPIC -std=legacy
# export BOOST_ROOT=$(TOP_DIR)

# export CUDA_CXX=nvcc -arch=sm_20

ifdef USE_OPENMP
  export CXX=smpicxx -std=c++14 -fopenmp $(CFLAGS)  $(OPT_DEFINES)
  export F77=gfortran -fopenmp $(FFLAGS) 
else
  export CXX=smpicxx $(CFLAGS) $(OPT_DEFINES)
  export F77=gfortran $(FFLAGS)
endif

# JSON++ requires flex and bison (version>=2.4)
export FLEX=flex
export BISON=/usr/local/opt/bison/bin/bison

export LUACXX = $(CXX)
export CC = gcc $(CFLAGS)
export F90 = gfortran $(FFLAGS)
