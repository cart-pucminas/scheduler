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

#
# Program arguments.
#   $1: Number of tasks.
#   $2: Number of threads.
#   $3: Input directory.
#   $4: Output directory.
#
NTASKS=$1   # Number of tasks.
NTHREADS=$2 # Number of threads.
INDIR=$3    # Input directory.
OUTDIR=$4   # Output directory.

# Directories.
BINDIR=$PWD/bin

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
	grep "Total Cycles" $1.tmp \
	| cut -d" " -f 3           \
	>> $OUTDIR/$1-cycles.csv
	
	grep "Thread" $1.tmp       \
	| cut -d" " -f 3           \
	>> $OUTDIR/$1-workload.csv
}

#
# Parses the benchmark.
#  $1 Scheduling strategy.
#  $2 Number of threads.
#  $3 Workload.
#  $4 Skewness.
#  $5 Kernel.
#  $6 Sorting
#
function parse_benchmark
{
	extract_variables benchmark-$3-$4-$6-$NTASKS-$5-$1-$2
}

#===============================================================================
#                                 MAIN ROUTINE
#===============================================================================

# Create directories.
mkdir -p $OUTDIR

for workload in "${WORKLOAD[@]}";
do
	for skewness in "${SKEWNESS[@]}";
	do
		for kernel in "${KERNELS[@]}";
		do
			for sorting in "${SORT[@]}";
			do
				$BINDIR/searcher                                           \
					--input $INDIR/$workload-$NTASKS-$skewness-$kernel.csv \
					--ntasks $NTASKS                                       \
					--nthreads $NTHREADS                                   \
					--crossover 0.80                                       \
					--mutation 0.10                                        \
					--replacement 0.90                                     \
					--elitism 0.01                                         \
					--popsize 1000                                         \
					--ngen 100                                             \
				1> benchmark-$workload-$skewness-$sorting-$NTASKS-$kernel-ga-$NTHREADS.tmp
				parse_benchmark ga $NTHREADS $workload $skewness $kernel $sorting
					
				# House keeping.
				rm -f benchmark-$workload-$skewness-$sorting-$NTASKS-$kernel-ga-$NTHREADS.tmp
			done
		done
	done
done

for pdf in "${WORKLOAD[@]}";
do
	# Move files.
	mkdir -p $OUTDIR/$pdf/cycles $OUTDIR/$pdf/workload
	mv $OUTDIR/*-$pdf-*-cycles.csv $OUTDIR/$pdf/cycles
	
	for kernel in "${KERNELS[@]}";
	do
		for sorting in "${SORT[@]}";
		do		
			# Header.
			echo ${SKEWNESS[@]}                                                              \
				> $OUTDIR/$pdf/cycles/benchmark-$pdf-$sorting-$NTASKS-$kernel-ga-$NTHREADS-cycles.csv
					
			# Data.
			paste -d " "                                                                         \
				$OUTDIR/$pdf/cycles/benchmark-$pdf-0.???-$sorting-$NTASKS-$kernel-ga-$NTHREADS-cycles.csv \
				>> $OUTDIR/$pdf/cycles/benchmark-$pdf-$sorting-$NTASKS-$kernel-ga-$NTHREADS-cycles.csv
		done
	done
done
