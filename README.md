# Simgrid Install and Test

## Install

```bash
cd SimGrid-3.26
mkdir build
cd build
cmake .. -Denable_documentation=OFF -DCMAKE_INSTALL_PREFIX=/opt/simgrid3.26
make
sudo make install
make tests
ctest
vim ~/.bashrc (add export PATH=$PATH:/opt/simgrid3.26/bin)
source ~/.bashrc
```
## Test

### Example 1: roundtrip
```bash
cd simgrid-template-smpi/
smpicc -O3 roundtrip.c -o roundtrip
smpirun -np 16 -platform cluster_crossbar.xml -hostfile cluster_hostfile [--cfg=smpi/display-timing:yes] ./roundtrip
```

### Example 2: time dependent trace
```bash
cd simgrid-template-smpi/NPB3.3-MPI/
make lu NPROCS=4 CLASS=S
smpirun -np 4 -platform ../cluster_backbone.xml -trace --cfg=tracing/filename:lu.S.4.trace bin/lu.S.4
Rscript draw_gantt.R lu.S.4.trace
```

### Example 3: time independent trace
```bash
cd simgrid-template-smpi/NPB3.3-MPI/
make lu NPROCS=8 CLASS=A
smpirun -np 8 -platform ../cluster_backbone.xml -trace-ti --cfg=tracing/filename:LU.A.8 bin/lu.A.8
smpirun -np 8 -platform ../cluster_crossbar.xml -hostfile ../cluster_hostfile -replay LU.A.8
```

### Example 4: computation size (1000, 2000, 3000)
```bash
cd simgrid-template-smpi/
smpicxx -O3 gemm_mpi.cpp -o gemm
time smpirun -np 16 -platform cluster_crossbar.xml -hostfile cluster_hostfile --cfg=smpi/display-timing:yes --cfg=smpi/host-speed:1000000000 ./gemm
```

