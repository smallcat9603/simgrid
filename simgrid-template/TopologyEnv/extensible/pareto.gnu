
set term x
#set term png font "arial,11"
set output "pareto.png"
set datafile separator ';'

#plot 'pareto.csv' using 4:5 with points;

set xlabel 'Interconnect cost'
set ylabel 'Power'
plot for [IDX=0:21] 'pareto.csv' i IDX u 4:5 w linespoints title columnheader(1) lc IDX
pause -1
