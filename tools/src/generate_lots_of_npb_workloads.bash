#!/usr/bin/env bash

for (( i=1; i<=100; i++ ))
do
    ./generate_npb_workloads.py -n 2000 -p npb_profiles.json -o workload${i}.json -i 4
done
