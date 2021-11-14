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

# from /home/huyao/simgrid-template/MpiEnv/config/211014-fujitsu/platforms/
PLATFORMS = [
  'mesh-8-8',
  'mesh-4-4-4',
  'mesh-4-4-2-2',
  'torus-8-8',
  'torus-4-4-4',
  'torus-4-4-2-2',
  'n64d8r',
  'n64d8r-bisec',
  'n64d8r-aspl',
  'n64d8r-re',
  'n64d8r-bisec-re',
  'n64d8r-aspl-re',
  'n64d8r-re4',
  'n64d8r-bisec-re4',
  'n64d8r-aspl-re4',  
  ]

MPI = [
  'ompi',
  'mpich',
  'mvapich2',
]

class IPDPSSimGridTuner(MeasurementInterface):

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
    return manipulator

  def run(self, desired_result, input, limit):
    """
    Compile and run a given configuration then
    return performance
    """
    cfg = desired_result.configuration.data

    platform_file_prefix = '/home/huyao/simgrid-template/MpiEnv/config/211014-fujitsu/platforms/{0}'.format(cfg['platform'])

    run_cmd = '/home/huyao/simgrid-template/MpiEnv/simgrid/inst/bin/smpirun '
    run_cmd += '-np 64 '
    run_cmd += '-platform ' + platform_file_prefix + '.xml '
    run_cmd += '-hostfile ' + platform_file_prefix + '.txt '
    run_cmd += '--cfg=smpi/coll-selector:' + '{0} '.format(cfg['mpi'])
    run_cmd += 'your_execution_binary'

    print(run_cmd)
    
    run_result = self.call_program(run_cmd)
    assert run_result['returncode'] == 0
    assert 'SUCCESSFUL' in run_result['stdout']

    return Result(time=run_result['time']) # you may need to change according to what you want to read (hirasawa)
 
  def save_final_config(self, configuration):
    """called at the end of tuning"""
    print("Optimal result to mytest.json:", configuration.data)
    self.manipulator().save_to_file(configuration.data,
                                    'mytest.json')


if __name__ == '__main__':
  argparser = opentuner.default_argparser()
  IPDPSSimGridTuner.main(argparser.parse_args())
