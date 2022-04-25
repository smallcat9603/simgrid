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

# CT =[
#   0, # no compression
#   1, # bytewise compression
#   5, # bitwise compression
#   7, # bitmask compression
# ]

NW = [
  'btl openib,self', # Infiniband 100G
  'btl_tcp_if_include eth0', # Ethernet 10G
]

MAP_BY = [
  'node',
  'slot',
]

class TestSimGridTuner(MeasurementInterface):

  def manipulator(self):
    """
    Define the search space by creating a
    ConfigurationManipulator
    """
    manipulator = ConfigurationManipulator()
    manipulator.add_parameter(
      EnumParameter('nw', NW)
    )
    manipulator.add_parameter(
      EnumParameter('map_by', MAP_BY)
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
    run_cmd += '-np 8 -H calc09,calc10 '
    # run_cmd += '-hostfile '
    run_cmd += '-mca btl_openib_allow_ib true '
    run_cmd += '-mca ' + '{0} '.format(cfg['nw'])
    run_cmd += '--map-by ' + '{0} '.format(cfg['map_by'])
    if self.args.appname == "mm": # MM
      run_cmd += mpi_bench_dir + 'mm/mm'
    elif self.args.appname == "graph500": # graph500
      run_cmd += mpi_bench_dir + 'graph500/mpi/graph500_mpi_simple 16'
    elif self.args.appname == "himeno": # himeno
      run_cmd += mpi_bench_dir + 'himeno/bmt'
    elif self.args.appname == "kmeans": # k-means (modified for compression)
      run_cmd += mpi_bench_dir + 'kmeans/kmeans'
    # else: # NPB
    #   run_cmd += '../../../simgrid-template-smpi/NPB3.3-MPI/bin/' + self.args.appname

    print(run_cmd)
    
    run_result = self.call_program(run_cmd)
    assert run_result['returncode'] == 0
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
