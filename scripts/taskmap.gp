set terminal postscript eps enhanced 12
set output outputname
set yrange [0:]

set datafile separator ";"


set boxwidth 0.75
set style fill solid

set ylabel "Assigned Workload (%)'
set xlabel "Threads (id)"
set key left top
set title titlename
plot inputname using 2:xtic(1) with boxes title "Workload"
