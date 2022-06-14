#!/usr/bin/env python3
#
# Kashiwa-AutoTuner
#

import adddeps  # fix sys.path

import opentuner
from opentuner import ConfigurationManipulator
from opentuner import IntegerParameter
from opentuner import EnumParameter
from opentuner import MeasurementInterface
from opentuner import Result

# hosts
HOSTS = ['calc09', 'calc10', 'calc11', 'calc12', 'calc13', 'calc14', 'calc15', 'calc16']
num_hosts = len(HOSTS)
num_procs = 16
HOST_SLOTS = []

# network
NW = [
  'btl openib,self', # Infiniband 100G
  'btl_tcp_if_include eth0', # Ethernet 10G
]

# mapping 
MAP_BY = [
  'node',
  'slot',
]

# compression
CT =[
  0, # no compression
  1, # bytewise compression
  5, # bitwise compression
  7, # bitmask compression
]

# for HPL
Ns = [100000]
NBs = [256, 288, 320, 352, 384]
MAPPING = [0] #0=Row-major
Ps = [2]
Qs = [4]
THRESHOLD = [16.0]
PFACTs = [0, 1, 2] #0=left/1=Crout/2=Right
NBMINs = [1, 2, 4, 8]
NDIVs = [2, 3, 4]
RFACTs = [0, 1, 2] #0=left/1=Crout/2=Right
BCASTs = [0, 1, 2, 3, 4, 5] #0=1rg/1=1rM/2=2rg/3=2rM/4=Lng/5=LnM
DEPTHs = [0, 1]
SWAP = [1, 2] #1=long/2=mix
SWAPTHRESHOLD = [64]
L1 = [0] #0=transposed
U = [0] #0=transposed
EQUILIBRATION = 1 #1=yes
MEMALIGN = [4, 8]

# Top-down Recursive
# m = number of processes (slots), n = number of nodes, p = [node:processes]
def gen(m, n, p=[]):
    if n==0:
        if m==0:
            yield p
        else:
            return
    else:
        for i in range(m+1):
            yield from gen(m-i, n-1, p+[i])

class TestSimGridTuner(MeasurementInterface):

  def manipulator(self):
    """
    Define the search space by creating a
    ConfigurationManipulator
    """
    manipulator = ConfigurationManipulator()
    # get HOST_SLOTS
    for p in gen(num_procs, num_hosts):
      slots = [ str(i) for i in p]
      host_slots = zip(HOSTS, slots)
      host_slots_new = []
      for host_slot in host_slots:
          host_slots_new.append(':'.join(host_slot))
      HOST_SLOTS.append(','.join(host_slots_new))
    manipulator.add_parameter(
      EnumParameter('host_slots', HOST_SLOTS)
    )
    manipulator.add_parameter(
      EnumParameter('nw', NW)
    )
    manipulator.add_parameter(
      EnumParameter('map_by', MAP_BY)
    )
    manipulator.add_parameter(
      EnumParameter('ct', CT)
    )
    manipulator.add_parameter(
      EnumParameter('ns', Ns)
    )
    manipulator.add_parameter(
      EnumParameter('nbs', NBs)
    )
    manipulator.add_parameter(
      EnumParameter('mapping', MAPPING)
    )
    manipulator.add_parameter(
      EnumParameter('ps', Ps)
    )
    manipulator.add_parameter(
      EnumParameter('qs', Qs)
    )
    manipulator.add_parameter(
      EnumParameter('threshold', THRESHOLD)
    )
    manipulator.add_parameter(
      EnumParameter('pfacts', PFACTs)
    )
    manipulator.add_parameter(
      EnumParameter('nbmins', NBMINs)
    )
    manipulator.add_parameter(
      EnumParameter('ndivs', NDIVs)
    )
    manipulator.add_parameter(
      EnumParameter('rfacts', RFACTs)
    )
    manipulator.add_parameter(
      EnumParameter('bcasts', BCASTs)
    )
    manipulator.add_parameter(
      EnumParameter('depths', DEPTHs)
    )
    manipulator.add_parameter(
      EnumParameter('swap', SWAP)
    )
    manipulator.add_parameter(
      EnumParameter('swapthreshold', SWAPTHRESHOLD)
    )
    manipulator.add_parameter(
      EnumParameter('l1', L1)
    )
    manipulator.add_parameter(
      EnumParameter('u', U)
    )
    manipulator.add_parameter(
      EnumParameter('equilibration', EQUILIBRATION)
    )
    manipulator.add_parameter(
      EnumParameter('memalign', MEMALIGN)
    )
    return manipulator

  def run(self, desired_result, input, limit):
    """
    Compile and run a given configuration then
    return performance
    """
    cfg = desired_result.configuration.data

    mpi_bench_dir = '/home/huyao/mpi/bench/'

    run_cmd = '/home/proj/atnw/local/bin/mpirun '
    run_cmd += '-np ' + str(num_procs) + ' '
    run_cmd += '-H ' + '{0} '.format(cfg['host_slots'])
    # run_cmd += '-hostfile '
    run_cmd += '-mca btl_openib_allow_ib true '
    if self.args.appname == "kmeans": # not work well at Ethernet NW
      run_cmd += '-mca btl openib,self '
    else:
      run_cmd += '-mca ' + '{0} '.format(cfg['nw'])
    run_cmd += '--map-by ' + '{0} '.format(cfg['map_by'])
    if self.args.appname == "mm": # MM
      run_cmd += mpi_bench_dir + 'mm/mm'
    elif self.args.appname == "graph500": # graph500
      run_cmd += mpi_bench_dir + 'graph500/mpi/graph500_mpi_simple 16'
    elif self.args.appname == "himeno": # himeno
      run_cmd += mpi_bench_dir + 'himeno/bmt'
    elif self.args.appname == "kmeans": # k-means (modified for compression)
      run_cmd += mpi_bench_dir + 'kmeans/kmeans' + ' {0}'.format(cfg['ct'])
    elif self.args.appname == "hpl": # hpl-2.3
      # for HPL (update HPL.dat)
      hpl_dat = []
      with open(mpi_bench_dir+"hpl-2.3/bin/ompi/HPL.dat", "r") as f:
        hpl_dat = f.readlines()
      hpl_dat[5] = '{0}\t\t\t Ns\n'.format(cfg['ns'])
      hpl_dat[7] = '{0}\t\t\t NBs\n'.format(cfg['nbs'])
      hpl_dat[8] = '{0}\t\t\t PMAP process mapping (0=Row-,1=Column-major)\n'.format(cfg['mapping'])
      hpl_dat[10] = '{0}\t\t\t Ps\n'.format(cfg['ps'])
      hpl_dat[11] = '{0}\t\t\t Qs\n'.format(cfg['qs'])
      hpl_dat[12] = '{0}\t\t\t threshold\n'.format(cfg['threshold'])
      hpl_dat[14] = '{0}\t\t\t PFACTs (0=left, 1=Crout, 2=Right)\n'.format(cfg['pfacts'])
      hpl_dat[16] = '{0}\t\t\t NBMINs (>= 1)\n'.format(cfg['nbmins'])
      hpl_dat[18] = '{0}\t\t\t NDIVs\n'.format(cfg['ndivs'])
      hpl_dat[20] = '{0}\t\t\t RFACTs (0=left, 1=Crout, 2=Right)\n'.format(cfg['rfacts'])
      hpl_dat[22] = '{0}\t\t\t BCASTs (0=1rg,1=1rM,2=2rg,3=2rM,4=Lng,5=LnM)\n'.format(cfg['bcasts'])
      hpl_dat[24] = '{0}\t\t\t DEPTHs (>=0)\n'.format(cfg['depths'])
      hpl_dat[25] = '{0}\t\t\t SWAP (0=bin-exch,1=long,2=mix)\n'.format(cfg['swap'])
      hpl_dat[26] = '{0}\t\t\t swapping threshold\n'.format(cfg['swapthreshold'])
      hpl_dat[27] = '{0}\t\t\t L1 in (0=transposed,1=no-transposed) form\n'.format(cfg['l1'])
      hpl_dat[28] = '{0}\t\t\t U  in (0=transposed,1=no-transposed) form\n'.format(cfg['u'])
      hpl_dat[29] = '{0}\t\t\t Equilibration (0=no,1=yes)\n'.format(cfg['equilibration'])
      hpl_dat[30] = '{0}\t\t\t memory alignment in double (> 0)\n'.format(cfg['memalign'])
      with open(mpi_bench_dir+"hpl-2.3/bin/ompi/HPL.dat", "w") as f:
        f.write("".join(hpl_dat)) 
      run_cmd = '/home/proj/atnw/local/bin/mpirun -np 8 -npernode 1 -H calc09,calc10,calc11,calc12,calc13,calc14,calc15,calc16 -mca btl_openib_allow_ib true -mca btl openib,self -x OMP_NUM_THREADS=10 -x PATH -x LD_LIBRARY_PATH taskset -c 0-9 ' + mpi_bench_dir + 'hpl-2.3/bin/ompi/xhpl'
    # else: # NPB
    #   run_cmd += '../../../simgrid-template-smpi/NPB3.3-MPI/bin/' + self.args.appname

    print(run_cmd)
    
    run_result = self.call_program(run_cmd)
    # assert run_result['returncode'] == 0
    # assert 'SUCCESSFUL' in run_result['stdout']

    return Result(time=run_result['time']) 
 
  def save_final_config(self, configuration):
    """called at the end of tuning"""
    file_name = self.args.appname + ".json"
    print("Optimal result to " + file_name + ":", configuration.data)
    self.manipulator().save_to_file(configuration.data,
                                    file_name)

if __name__ == '__main__':
  argparser = opentuner.default_argparser()
  argparser.add_argument('-a', '--appname', required=True,
                       help="application name to be tuned")
  TestSimGridTuner.main(argparser.parse_args())
