set datafile separator ";"

set terminal postscript eps enhanced color 12
set output outputname

plot inputname using 1:3 with lines lc 'blue' title columnheader,\
      inputname using 1:4 with lines lc 'red' title columnheader
