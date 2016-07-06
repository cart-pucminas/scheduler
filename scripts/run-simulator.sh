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

#
# Program arguments.
#   $1: Number of threads.
#   $2: Number of loop iterations.
#
NTHREADS=$1    # Number of threads.
NITERATIONS=$2 # Number of iterations.

# Directories.
BINDIR=$PWD/bin
CSVDIR=$PWD/csv

# Kernel type.
KERNEL_TYPE=linear

# Scheduling strategies.
STRATEGIES=(workload-aware dynamic srr)

# Workloads.
WORKLOAD=(gaussian)

# Workload sorting.
SORT=(random)

# Skewness
SKEWNESS=(0.750 0.775 0.800 0.825 0.850 0.875 0.900)

#===============================================================================
#                              PARSING ROUTINES
#===============================================================================

#
# Extracts variables from raw results.
#   $1 Filename prefix.
#
function extract_variables
{	
	grep "Total Cycles" $1.tmp \
	| cut -d" " -f 3           \
	>> $CSVDIR/$1-cycles.csv
	
	grep "Thread" $1.tmp       \
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
#                                 RUN ROUTINES
#===============================================================================

#
# Run synthetic benchmark.
#  $1 Scheduling strategy.
#  $2 Number of threads.
#  $3 Workload.
#  $4 Skewness
#  $5 Sorting
#
function run_benchmark
{
	$BINDIR/scheduler \
		--kernel $KERNEL_TYPE      \
		--nthreads $2              \
		--niterations $NITERATIONS \
		--pdf $3                   \
		--skewness $4              \
		--sort $5                  \
		$1                         \
	2>> benchmark-$3-$4-$5-$NITERATIONS-$1-$2.tmp
}

#===============================================================================
#                                 MAIN ROUTINE
#===============================================================================

# Create directories.
mkdir -p $CSVDIR

for strategy in "${STRATEGIES[@]}";
do
	for skewness in "${SKEWNESS[@]}";
	do
		for workload in "${WORKLOAD[@]}";
		do
			for sorting in "${SORT[@]}";
			do
				run_benchmark $strategy $NTHREADS $workload $skewness $sorting
				parse_benchmark $strategy $NTHREADS $workload $skewness $sorting
				
				# House keeping.
				rm -f *.tmp
			done
		done
	done
done

for pdf in "${WORKLOAD[@]}";
do

	# Move files.
	mkdir -p $CSVDIR/$pdf/cycles $CSVDIR/$pdf/workload
	mv $CSVDIR/*-$pdf-*-cycles.csv $CSVDIR/$pdf/cycles
	mv $CSVDIR/*-$pdf-*-workload.csv $CSVDIR/$pdf/workload
	
	for strategy in "${STRATEGIES[@]}";
	do
		for sorting in "${SORT[@]}";
		do
			# Header.
			echo ${SKEWNESS[@]} \
				> $CSVDIR/$pdf/cycles/benchmark-$pdf-$sorting-$NITERATIONS-$strategy-$NTHREADS-cycles.csv
			
			# Data.
			paste -d " " \
				$CSVDIR/$pdf/cycles/benchmark-$pdf-0.???-$sorting-$NITERATIONS-$strategy-$NTHREADS-cycles.csv \
				>> $CSVDIR/$pdf/cycles/benchmark-$pdf-$sorting-$NITERATIONS-$strategy-$NTHREADS-cycles.csv
			
			# House keeping.
			rm -f $CSVDIR/$pdf/cycles/benchmark-$pdf-0.???-$sorting-$NITERATIONS-$strategy-$NTHREADS-cycles.csv
		done
	done
done
