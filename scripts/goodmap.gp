set datafile separator ";"

stats inputname2 nooutput

inputname2_max = STATS_max_y
inputname2_min = STATS_min_y

f(x) = 1.0/(inputname2_max - inputname2_min)

stats inputname3 nooutput

inputname3_max = STATS_max_y
inputname3_min = STATS_min_y

g(x) = 1.0/(inputname3_max - inputname3_min)

set terminal postscript eps enhanced color 12
set output outputname

set xlabel "Number of Generations"
set ylabel "Load Balancing Fitness"
set title titlename

set key opaque bottom right box

plot inputname using 1:4 with lines lt rgb "blue" linewidth 2 title 'Genetic Algorithm',\
     f(x) with lines lt rgb "red" linewidth 2 title 'Static',\
     g(x) with lines lt rgb "#242424" linewidth 2 title 'Dynamic'\
