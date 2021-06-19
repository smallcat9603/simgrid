c NPROCS = 4 CLASS = S
c  
c  
c  This file is generated automatically by the setparams utility.
c  It sets the number of processors and the class of the NPB
c  in this directory. Do not modify it by hand.
c  

c number of nodes for which this version is compiled
        integer nnodes_compiled, nnodes_xdim
        parameter (nnodes_compiled=4, nnodes_xdim=2)

c full problem size
        integer isiz01, isiz02, isiz03
        parameter (isiz01=12, isiz02=12, isiz03=12)

c sub-domain array size
        integer isiz1, isiz2, isiz3
        parameter (isiz1=6, isiz2=6, isiz3=isiz03)

c number of iterations and how often to print the norm
        integer itmax_default, inorm_default
        parameter (itmax_default=50, inorm_default=50)
        double precision dt_default
        parameter (dt_default = 0.5d0)
        logical  convertdouble
        parameter (convertdouble = .false.)
        character*11 compiletime
        parameter (compiletime='24 Oct 2018')
        character*5 npbversion
        parameter (npbversion='3.3.1')
        character*26 cs1
        parameter (cs1='${SIMGRID_PATH}/bin/smpiff')
        character*9 cs2
        parameter (cs2='$(MPIF77)')
        character*37 cs3
        parameter (cs3='-lsimgrid -lgfortran #-lsmpi -lgras  ')
        character*31 cs4
        parameter (cs4='-I${SIMGRID_PATH}/include/smpi/')
        character*2 cs5
        parameter (cs5='-O')
        character*25 cs6
        parameter (cs6='-O  -L${SIMGRID_PATH}/lib')
        character*6 cs7
        parameter (cs7='randi8')
