#!/bin/sh

for nodes in 256
do
    for degree in 4 8
    do
        for topo in g r
        do
            for prog in bt cg ep ft is lu mg sp
            do
                ../simgrid-template/MpiEnv/simgrid/inst/bin/smpirun -np ${nodes} --cfg=smpi/privatize_global_variables:yes -platform platforms/n${nodes}d${degree}${topo}.xml -hostfile platforms/n${nodes}d${degree}${topo}.txt ../simgrid-template/MpiEnv/bench/NPB3.3.1/NPB3.3-MPI/bin/${prog}.A.${nodes} | tee results/${prog}_n${nodes}d${degree}${topo}.txt
            done
        done
    done
done