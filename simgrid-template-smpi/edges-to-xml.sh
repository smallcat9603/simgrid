#!/bin/sh

# by smallcat
# for simgrid 3.2X use 

input="./topologies/size_64/"
output="./platforms/"

files=$(ls ${input})
for file in ${files}
do
  ruby ../tools/src/generate_platform.rb ${input}${file} ${output}${file%.*}
done