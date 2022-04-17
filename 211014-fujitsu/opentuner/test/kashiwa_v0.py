#!/usr/bin/env python
#
# ipdps-simgrid-Tuner
#
import adddeps  # fix sys.path

import opentuner
from opentuner import ConfigurationManipulator
from opentuner import IntegerParameter
from opentuner import EnumParameter
from opentuner import MeasurementInterface
from opentuner import Result

# from platforms/
PLATFORMS = [
  'mesh-8-8', 'mesh-4-4-4', 'mesh-4-4-2-2',
  'torus-8-8', 'torus-4-4-4', 'torus-4-4-2-2',
  'n64d8r', 'n64d8r.bisec', 'n64d8r.aspl', 
  'n64d8r.re1', 'n64d8r.bisec.re1', 'n64d8r.aspl.re1',
  'n64d8r.re4', 'n64d8r.bisec.re4', 'n64d8r.aspl.re4',  
  'n64d6r', 'n64d6r.bisec', 'n64d6r.aspl',
  'n64d6r.re1', 'n64d6r.bisec.re1', 'n64d6r.aspl.re1',
  'n64d6r.re4', 'n64d6r.bisec.re4', 'n64d6r.aspl.re4',  
  'n64d4r', 'n64d4r.bisec', 'n64d4r.aspl',
  'n64d4r.re1', 'n64d4r.bisec.re1', 'n64d4r.aspl.re1',
  'n64d4r.re4', 'n64d4r.bisec.re4', 'n64d4r.aspl.re4',    
  ]

MPI = [
  'ompi',
  'mpich',
  'mvapich2',
]

ALLTOALL = [
  'bruck',
  '2dmesh',
  '3dmesh',
  'rdb',
  'pair',
  'ring',
  'basic_linear',
]

ALLREDUCE = [
  'rdb',
  'lr',
  'rab1',
  'rab2',
  # 'rab_rsag',
  'rab',
]

ALLGATHER =[
  '2dmesh',
  '3dmesh',
  'bruck',
  'pair',
  'rdb',
  'ring',
  'spreading_simple',
]

BCAST =[
  'binomial_tree',
  'flattree',
  # 'scatter_LR_allgather',
  'scatter_rdb_allgather',
]

CT =[
  0, # no compression
  1, # bytewise compression
  5, # bitwise compression
  7, # bitmask compression
]

class TestSimGridTuner(MeasurementInterface):

  def manipulator(self):
    """
    Define the search space by creating a
    ConfigurationManipulator
    """
    manipulator = ConfigurationManipulator()
    manipulator.add_parameter(
      EnumParameter('platform', PLATFORMS)
    )
    manipulator.add_parameter(
      EnumParameter('mpi', MPI)
    )
    manipulator.add_parameter(
      EnumParameter('alltoall', ALLTOALL)
    )
    manipulator.add_parameter(
      EnumParameter('allreduce', ALLREDUCE)
    )
    manipulator.add_parameter(
      EnumParameter('allgather', ALLGATHER)
    )
    manipulator.add_parameter(
      EnumParameter('bcast', BCAST)
    )
    manipulator.add_parameter(
      EnumParameter('ct', CT)
    )
    return manipulator

  def run(self, desired_result, input, limit):
    """
    Compile and run a given configuration then
    return performance
    """
    cfg = desired_result.configuration.data

    platform_file_prefix = 'platforms/{0}'.format(cfg['platform'])

    run_cmd = '/opt/simgrid3.26/bin/smpirun '
    run_cmd += '-np 64 '
    run_cmd += '-platform ' + platform_file_prefix + '.xml '
    run_cmd += '-hostfile ' + platform_file_prefix + '.txt '
    # run_cmd += '--cfg=smpi/privatize_global_variables:yes '
    run_cmd += '--cfg=smpi/coll-selector:' + '{0} '.format(cfg['mpi']) #coll_selector for simgrid 3.12, coll-selector for simgrid 3.2x
    run_cmd += '--cfg=smpi/alltoall:' + '{0} '.format(cfg['alltoall'])
    run_cmd += '--cfg=smpi/allreduce:' + '{0} '.format(cfg['allreduce'])
    run_cmd += '--cfg=smpi/allgather:' + '{0} '.format(cfg['allgather'])
    run_cmd += '--cfg=smpi/bcast:' + '{0} '.format(cfg['bcast'])
    if self.args.appname == "gemm": # MM
      run_cmd += '../../../simgrid-template-smpi/' + self.args.appname
    elif self.args.appname == "graph500_mpi_simple": # graph500
      run_cmd += '../../../simgrid-template/MpiEnv/bench/graph500/graph500/mpi/' + self.args.appname + " 64 8"
    elif self.args.appname == "himeno": # himeno
      run_cmd += '../../../simgrid-template/MpiEnv/bench/himeno/bin/' + self.args.appname
    elif self.args.appname == "kmeans_simgrid": # k-means (modified for compression)
      run_cmd += '../../../../data-compression/impl/' + self.args.appname + ' {0}'.format(cfg['ct'])
    else: # NPB
      run_cmd += '../../../simgrid-template-smpi/NPB3.3-MPI/bin/' + self.args.appname

    print(run_cmd)
    
    run_result = self.call_program(run_cmd)
    assert run_result['returncode'] == 0
    # assert 'SUCCESSFUL' in run_result['stdout']

    return Result(time=run_result['stime']) # simulated time for simgrid (added new objective in interface.py)
 
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
