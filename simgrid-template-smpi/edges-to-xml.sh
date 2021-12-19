#!/bin/sh

# by smallcat
# for simgrid 3.2X use 

input="./topologies/size_64/"
output="./platforms/"

files=$(ls ${input})
for file in ${files}
do
  ruby ../tools/src/generate_platform.rb ${input}${file} ${file%.*}
done

# prog=pingpong #pingpong mm lu k-means
# procs=2
# mmpara="testdata/mat_512_512_a.txt testdata/mat_512_512_b.txt"
# lupara=256


# for CT in 10 #0 5 8 10
# do
#     for AEB in 0.000001 #0.000001 0.00001 0.0001 0.001 0.01
#     do
#         for BER in 1e-6 #1e-16 1e-10 1e-9 1e-8 1e-7 1e-6 1e-5
#         do
#             sh ./set-parameter.sh dataCompression.h ${CT} ${AEB} ${BER}
#             mpicc ${prog}.c dataCompression.c -o ${prog} -lm -lz
#             mpirun -np ${procs} ./${prog} #${lupara}
#         done
#     done 
# done