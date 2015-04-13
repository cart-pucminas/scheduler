
reset

stats inputname using 1 nooutput

n=16                           # number of intervals
max=STATS_max                  # max value
min=STATS_min                  # min value
width=(max-min)/n              # interval width

# Function used to map a value to the intervals
hist(x,width)=width*floor(x/width)+width/2.0

set terminal postscript eps enhanced 12
set output outputname
set xrange [min:max]
set yrange [0:]

#to put an empty boundary around the
#data inside an autoscaled graph.
set offset graph 0.05,0.02,0.05,0.0
set xtics min,(max-min)/8,max
set boxwidth width*0.9
set grid ytics linestyle 0
set style fill solid 0.50 border 2              #fillstyle
set tics out nomirror
set xlabel "Tasks Size"
set ylabel "Frequency"

#count and plot
set key left top
plot inputname u (hist($1,width)):(1.0) title "task" smooth freq w boxes lc rgb"green"

