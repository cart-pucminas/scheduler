set terminal postscript eps enhanced 12
set output outputname

set datafile separator ";"


set boxwidth 0.75
set style fill solid

set key left top
plot inputname using 2:xtic(1) with boxes title "Workload"
