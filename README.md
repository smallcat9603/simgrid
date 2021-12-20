# SimGrid Install and Test

## Install SimGrid

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
## Test SimGrid

### master-workers
```bash
cd simgrid-template-s4u/
export SimGrid_PATH=/opt/simgrid3.26
cmake -D SimGrid_PATH=/opt/simgrid3.26 .
make
./master-workers small_platform.xml master-workers_d.xml [2>&1 | simgrid-colorizer]
./master-workers small_platform.xml master-workers_d.xml --cfg=tracing:yes --cfg=tracing/actor:yes
Rscript draw_gantt.R simgrid.trace
```

### Example 1: roundtrip
```bash
cd simgrid-template-smpi/
smpicc -O3 roundtrip.c -o roundtrip
smpirun -np 16 -platform cluster_crossbar.xml -hostfile cluster_hostfile [--cfg=smpi/display-timing:yes] ./roundtrip
```

### Example 2: time dependent trace (NPB)
```bash
cd simgrid-template-smpi/NPB3.3-MPI/
make lu NPROCS=4 CLASS=S
smpirun -np 4 -platform ../cluster_backbone.xml -trace --cfg=tracing/filename:lu.S.4.trace bin/lu.S.4
Rscript draw_gantt.R lu.S.4.trace
```

### Example 3: time independent trace (NPB)
```bash
cd simgrid-template-smpi/NPB3.3-MPI/
make lu NPROCS=8 CLASS=A
smpirun -np 8 -platform ../cluster_backbone.xml -trace-ti --cfg=tracing/filename:LU.A.8 bin/lu.A.8
smpirun -np 8 -platform ../cluster_crossbar.xml -hostfile ../cluster_hostfile -replay LU.A.8
```

### Example 4: computation size (MM) (1000, 2000, 3000)
```bash
cd simgrid-template-smpi/
smpicxx -O3 gemm_mpi.cpp -o gemm
time smpirun -np 16 -platform cluster_crossbar.xml -hostfile cluster_hostfile --cfg=smpi/display-timing:yes --cfg=smpi/host-speed:1000000000 ./gemm
```
## Example 5: Compile and Run Himeno
```bash
cd simgrid-template/MpiEnv/bench/himeno/
sh paramset.sh S 4 4 4 # for 64 procs (p= X x Y x Z)
# modify config/make.def for simgrid path, like SIMGRID_PATH=/opt/simgrid3.26/
make
smpirun -np 64 -platform <PLATFORM> -hostfile <HOSTFILE> bin/himeno
```

## Example 6: Compile and Run Graph500
```bash
cd simgrid-template/MpiEnv/bench/graph500/graph500/mpi/
# modify make.def for simgrid path, like SIMGRID_PATH=/opt/simgrid3.26/
# modify Makefile for smpicc path, like MPICC = /opt/simgrid3.26/bin/smpicc
make
smpirun -np 64 -platform <PLATFORM> -hostfile <HOSTFILE> ./graph500_mpi_simple 64 8
```

## Example 6: Compile and Run HPL
```bash
cd SMPI-proxy-apps/
# download hpl-2.3
cd hpl-2.3
cp ../src/HPL/Make.SMPI .
mkdir -p bin/SMPI
cp ../src/HPL/HPL.dat bin/SMPI
# modify Make.SMPI for TOPdir path, like TOPdir=/github/simgrid/SMPI-proxy-apps/hpl-2.3, or run "sed -ri "s|TOPdir\s*=.+|TOPdir="`pwd`"|g" Make.SMPI"
make startup arch=SMPI
make arch=SMPI
cd bin/SMPI
smpirun -np 16 -hostfile ../src/common/cluster_hostfile.txt -platform ../src/common/cluster_crossbar.xml ./xhpl
```

## Example 7: Run OpenTuner
```bash
# download opentuner
# replace jansel-opentuner/opentuner/measurement/interface.py
python mytest.py -a himeno --no-dups --stop-after=600 --database himeno.64.db --results-log-details himeno.64.log
```

## Install Batsim
```bash
curl -L https://nixos.org/nix/install | sh
# Follow the instructions displayed at the end of the script.
. /home/smallcat/.nix-profile/etc/profile.d/nix.sh
# Install the Batsim simulator.
nix-env -f https://github.com/oar-team/nur-kapack/archive/master.tar.gz -iA batsim
# Other packages from the Batsim ecosystem can also be installed this way.
# For example schedulers.
nix-env -f https://github.com/oar-team/nur-kapack/archive/master.tar.gz -iA batsched
nix-env -f https://github.com/oar-team/nur-kapack/archive/master.tar.gz -iA pybatsim
# Or interactive visualization tools.
# nix-env -f https://github.com/oar-team/nur-kapack/archive/master.tar.gz -iA evalys
# Or experiment management tools...
nix-env -f https://github.com/oar-team/nur-kapack/archive/master.tar.gz -iA batexpe
# Check installation
batsim --version
batsim --simgrid-version
batsched --version
robin --version
```

## Test Batsim
```bash
robin ./expe4.yaml
# Add batsim to PATH
vim ~/.bashrc (add export PATH=$PATH:/home/smallcat/.nix-profile/bin)
source ~/.bashrc
```

## Tools
### generate host file
```bash
python generate_host_file.py
```

### generate edge file
```bash
ruby lay4.rb [-c] 64r4 DLN-64-2-5.edges //64 nodes, 4 nodes/rack, 16 racks, nodes are connected by a specified edge file
ruby lay4.rb [-c] 64r1 torus-4-4-4 //64 nodes, 1 node/rack, 64 racks, nodes are connected by a predefined topology (4*4*4 torus)
```

### generate platform file (for simgrid use)
```bash
ruby generate_platform.rb torus-4-4-4.edges torus-4-4-4 //with config.rb in the same relative path
```

### generate master node (for batsim edge file use)
```bash
python generate_random_master.py
```

### generate delay profiles (for batsim use)
```bash
python generate_delay_profiles.py -o ../output/delays.json -i 4 [-n NB_DELAYS]
```

### generate stupid workload (for batsim use)
```bash
python generate_stupid_workloads.py -p delays.json -o ../output/stupid.json -i 4 [-n NB_JOBS] [--nb-res NB_RES]
```

### generate npb workload (for batsim use)
```bash
python generate_npb_workloads.py -p npb_profiles.json -o ../output/npb.json -i 4 [-n NB_JOBS] [--nb-res NB_RES]
```
