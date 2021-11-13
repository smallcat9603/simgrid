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

# from /home/huyao/simgrid-template/MpiEnv/config/211003-ipdps/platforms/
PLATFORMS = [
  '2_n256d4g.edges.re',
  '2_n256d4g',
  '2_n256d8g.edges.re',
  '2_n256d8g',
  'n256d4g.edges.re',
  'n256d8g.edges.re',
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
    # if you have more parameters, add below (hirasawa)
    #manipulator.add_parameter(
    #  EnumParameter('bin', BINS)
    #)
    return manipulator

  def run(self, desired_result, input, limit):
    """
    Compile and run a given configuration then
    return performance
    """
    cfg = desired_result.configuration.data

    platform_file_prefix = '/home/huyao/simgrid-template/MpiEnv/config/211003-ipdps/platforms/{0}'.format(cfg['platform'])

    run_cmd = '/home/huyao/simgrid-template/MpiEnv/simgrid/inst/bin/smpirun '
    run_cmd += '-np 256 '
    run_cmd += '-platform ' + platform_file_prefix+'.xml '
    run_cmd += '-hostfile ' + platform_file_prefix+'.txt '
    run_cmd += 'your_execution_binary'

    print(run_cmd)
    
    run_result = self.call_program(run_cmd)
    assert run_result['returncode'] == 0
    assert 'SUCCESSFUL' in run_result['stdout']

    return Result(time=run_result['time']) # you may need to change according to what you want to read (hirasawa)
 
  def save_final_config(self, configuration):
    """called at the end of tuning"""
    print("Optimal result to ipdps_simgrid.json:", configuration.data)
    self.manipulator().save_to_file(configuration.data,
                                    'ipdps_simgrid.json')


if __name__ == '__main__':
  argparser = opentuner.default_argparser()
  IPDPSSimGridTuner.main(argparser.parse_args())
