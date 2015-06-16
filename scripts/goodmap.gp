set datafile separator ";"

stats inputname2 nooutput

inputname2_max = STATS_max_y
inputname2_min = STATS_min_y

f(x) = 1.0/(inputname2_max - inputname2_min)

stats inputname3 nooutput

inputname3_max = STATS_max_y
inputname3_min = STATS_min_y

g(x) = 1.0/(inputname3_max - inputname3_min)

stats inputname4 nooutput

inputname4_max = STATS_max_y
inputname4_min = STATS_min_y

h(x) = 1.0/(inputname4_max - inputname4_min)

stats inputname5 nooutput

inputname5_max = STATS_max_y
inputname5_min = STATS_min_y

i(x) = 1.0/(inputname5_max - inputname5_min)

stats inputname6 nooutput

inputname6_max = STATS_max_y
inputname6_min = STATS_min_y

j(x) = 1.0/(inputname6_max - inputname6_min)

stats inputname7 nooutput

inputname7_max = STATS_max_y
inputname7_min = STATS_min_y

k(x) = 1.0/(inputname7_max - inputname7_min)

set terminal postscript eps enhanced color 12
set output outputname

set xlabel "Number of Generations"
set ylabel "Load Balancing Fitness"
set title titlename

set key opaque bottom right box

plot inputname using 1:4 with lines lt rgb "blue" linewidth 2 title 'Genetic Algorithm',\
     f(x) with lines lt rgb "red" linewidth 2 title 'Static',\
     g(x) with lines lt rgb "#242424" linewidth 2 title 'Dynamic (chunksize = 1)',\
     h(x) with lines lt rgb "#A0522D" linewidth 2 title 'Dynamic (chunksize = 2)',\
     i(x) with lines lt rgb "#FFA500" linewidth 2 title 'Dynamic (chunksize = 4)',\
     j(x) with lines lt rgb "#3CB371" linewidth 2 title 'Dynamic (chunksize = 8)',\
     k(x) with lines lt rgb "#FF69B4" linewidth 2 title 'Dynamic (chunksize = 16)'
