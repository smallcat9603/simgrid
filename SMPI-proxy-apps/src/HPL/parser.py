import re
import argparse
import sys
import math


def parse_stderr(filename):
    float_string = '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?'
    sim_time = 'The simulation took (?P<simulation>%s) seconds \(after parsing and platform setup\)' % float_string
    app_time = '(?P<application>%s) seconds were actual computation of the application' % float_string
    full_time = 'Simulated time: (?P<full_time>%s) seconds.' % float_string
    smpi_reg = re.compile('[\S\s]*%s[\S\s]*%s\n%s' % (full_time, sim_time, app_time))

    with open(filename) as f:
        content = f.read()
    match = smpi_reg.search(content)
    simulation_time = float(match.group('simulation'))
    application_time = float(match.group('application'))
    full_time = float(match.group('full_time'))
    return simulation_time, application_time, full_time


def assert_float_equal(name, args, real, epsilon, relative=True):
    expected = args.__getattribute__(name)
    if relative:
        isclose = math.isclose(real, expected, rel_tol=epsilon)
    else:
        isclose = math.isclose(real, expected, abs_tol=epsilon)
    if not isclose:
        sys.stderr.write('Error with %s\n' % name)
        sys.stderr.write('Expected value: %f\n' % expected)
        sys.stderr.write('Real value:     %f\n' % real)
        sys.exit(1)


def check_stderr(args):
    simulation_time, application_time, full_time = parse_stderr(args.filename)
    real_ratio = application_time / simulation_time
    assert_float_equal('application_time_ratio', args, real=real_ratio, epsilon=0.1, relative=False)
    assert_float_equal('full_time', args, real=full_time, epsilon=0.05)
    print('OK')


def parse_stdout(filename):
    with open(filename) as f:
        lines = f.readlines()
    for i, line in enumerate(lines):
        if line.strip() == 'T/V                N    NB     P     Q               Time                 Gflops':
            result = lines[i + 2].split()
            return float(result[-2]), float(result[-1])


def check_stdout(args):
    time, gflops = parse_stdout(args.filename)
    assert_float_equal('time', args, real=time, epsilon=0.05)
    assert_float_equal('gflops', args, real=gflops, epsilon=0.05)
    print('OK')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Parsing of HPL files')
    parser.add_argument('filename', type=str)
    sp = parser.add_subparsers(dest='check_type')
    sp.required = True
    sp_stderr = sp.add_parser('check_stderr')
    sp_stderr.add_argument('application_time_ratio', type=float)
    sp_stderr.add_argument('full_time', type=float)
    sp_stdout = sp.add_parser('check_stdout')
    sp_stdout.add_argument('time', type=float)
    sp_stdout.add_argument('gflops', type=float)
    args = parser.parse_args()
    if args.check_type == 'check_stderr':
        check_stderr(args)
    else:
        assert args.check_type == 'check_stdout'
        check_stdout(args)
