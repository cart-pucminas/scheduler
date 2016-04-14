reset
stats 'numbers.txt'
n=8
min=STATS_min
max=STATS_max

width=(max-min)/n #interval width

#function used to map a value to the intervals
hist(x,width)=width*floor(x/width)+width/2.0

set terminal postscript
set output 'foo.eps'
set xrange [0:max]

#count and plot
plot "numbers.txt" u (hist($1,width)):(1.0/STATS_records) smooth freq w boxes lc rgb"green" notitle
