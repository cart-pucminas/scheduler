# Skewness
#
# Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA.
# 

# Program arguments.
#   $1: Number of threads.
#   $2: Number of loop iterations.
#
NTHREADS=$1    # Number of threads.
NITERATIONS=$2 # Number of iterations.

# Directories.
BINDIR=$PWD/bin
INDIR=$PWD/input
CSVDIR=$PWD/csv

# Import some variables.
source scripts/var.sh

#===============================================================================
#                              PARSING ROUTINES
#===============================================================================

#
# Extracts variables from raw results.
#   $1 Filename prefix.
#
function extract_variables
{	
	grep "Total Cycles" $CSVDIR/$1.tmp \
	| cut -d" " -f 3           \
	>> $CSVDIR/$1-cycles.csv
	
	grep "Thread" $CSVDIR/$1.tmp       \
	| cut -d" " -f 3           \
	>> $CSVDIR/$1-workload.csv
}

#
# Parses the benchmark.
#  $1 Scheduling strategy.
#  $2 Number of threads.
#  $3 Workload.
#  $4 Skewness.
#  $5 Sorting
#
function parse_benchmark
{
	extract_variables benchmark-$3-$4-$5-$NITERATIONS-$1-$2
}

#===============================================================================
#                                 MAIN ROUTINE
#===============================================================================

# Create directories.
mkdir -p $CSVDIR

for workload in "${WORKLOAD[@]}";
do
	for skewness in "${SKEWNESS[@]}";
	do
	
		for kernel in "${KERNELS[@]}";
		do
			$BINDIR/searcher                                        \
				--input $INDIR/$workload-$NITERATIONS-$skewness-$kernel.csv \
				--ntasks $NITERATIONS                                        \
				--nthreads $NTHREADS                                         \
				--crossover 0.80                                             \
				--mutation 0.10                                              \
				--replacement 0.90                                           \
				--elitism 0.01                                               \
				--popsize 1000                                               \
				--ngen 10000                                                 \
			1> $CSVDIR/benchmark-$workload-$skewness-random-$NITERATIONS-ga-$NTHREADS.tmp
			parse_benchmark ga $NTHREADS $workload $skewness random
				
			# House keeping.
			rm -f $CSVDIR/benchmark-$workload-$skewness-random-$NITERATIONS-ga-$NTHREADS.tmp
		done
	done
done

for pdf in "${WORKLOAD[@]}";
do

	# Move files.
	mkdir -p $CSVDIR/$pdf/cycles $CSVDIR/$pdf/workload
	mv $CSVDIR/*-$pdf-*-cycles.csv $CSVDIR/$pdf/cycles
	
	# Header.
	echo ${SKEWNESS[@]} \
		> $CSVDIR/$pdf/cycles/benchmark-$pdf-random-$NITERATIONS-ga-$NTHREADS-cycles.csv
			
	# Data.
	paste -d " " \
		$CSVDIR/$pdf/cycles/benchmark-$pdf-0.???-random-$NITERATIONS-ga-$NTHREADS-cycles.csv \
		>> $CSVDIR/$pdf/cycles/benchmark-$pdf-random-$NITERATIONS-ga-$NTHREADS-cycles.csv
done
