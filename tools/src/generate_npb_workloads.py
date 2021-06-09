#!/usr/bin/env python3

"""Generate npb workloads."""
import argparse
import json
import random
import time


def generate_workload(nb_jobs, input_profiles, nb_res,
                      output_filename, indent):
    """Generate a workload and writes it into output_filename."""
    profile_json = json.load(open(input_profiles, 'r'))
    profile_names = [profile for profile in profile_json['profiles']]
    walltime = 4200

    """Generate a workload with Poisson distribution."""
    makespan = 20
    subtime = 0.0
    jobs = []
    localtime = time.asctime( time.localtime(time.time()) )

    for i in range(1, nb_jobs+1):
        profile = random.choice(profile_names)
        res = int(profile.split(".")[-1])
        subtime += random.expovariate(nb_jobs/float(makespan))
        job = {"id": i, "subtime": subtime, "walltime": walltime, "res": res, "profile": profile}
        jobs.append(job)

    content = { "version": 0,
                "command:": "",
                "date": localtime,
                "description": "workload with profile file for npb",

                "nb_res": nb_res,
            #    "jobs": [{"id": i, "subtime": subtime + random.expovariate(nb_jobs/float(makespan)), "walltime": walltime, "res": 1,
            #              "profile": random.choice(profile_names)}
            #             for i in range(1, nb_jobs + 1)
            #             ],
               "jobs": jobs,
               "profiles": profile_json['profiles']
               }

    with open(output_filename, 'w') as f:
        json.dump(content, f, indent=indent, sort_keys=True)


def main():
    """
    Program entry point.

    Parses the input arguments and call generate_workload.
    """
    script_description = "Generates a lot of npb profiles into a file."

    p = argparse.ArgumentParser(description=script_description)

    p.add_argument('-n', '--nb-jobs',
                   type=int,
                   default=2000,
                   help='The number of different delay profiles to generate')

    p.add_argument('-p', '--input-profiles',
                   type=str,
                   required=True,
                   help='The input json file containing profiles')

    p.add_argument('--nb-res',
                   type=int,
                   default=1024,
                   help='The number of resources')

    p.add_argument('-o', '--output',
                   type=str,
                   required=True,
                   help='The output file name')

    p.add_argument('-i', '--indent',
                   type=int,
                   default=None,
                   help='The output json indent')

    args = p.parse_args()
    generate_workload(args.nb_jobs, args.input_profiles, args.nb_res,
                      args.output, args.indent)

if __name__ == '__main__':
    main()
