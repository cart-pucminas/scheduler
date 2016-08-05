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
#   $5: Experiments design.
#
NTASKS=$1   # Number of tasks.
NTHREADS=$2 # Number of threads.
INDIR=$3    # Input directory.
OUTDIR=$4   # Output directory.
DESIGN=$5   # Experiments design.

# Directories.
BINDIR=$PWD/bin

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
#  $7 seed.
#
function parse_benchmark
{
	extract_variables benchmark-$3-$4-$6-$NTASKS-$7-$5-$1-$2
}

#===============================================================================
#                                 RUN ROUTINES
#===============================================================================

#
# Run synthetic benchmark.
#  $1 Scheduling strategy.
#  $2 Number of threads.
#  $3 Workload.
#  $4 Skewness.
#  $5 Kernel.
#  $6 Sorting.
#  $7 seed.
#
function run_benchmark
{	
	$BINDIR/scheduler                       \
		--input $INDIR/$3-$NTASKS-$4-$5.csv \
		--nthreads $2                       \
		--niterations $NTASKS               \
		--sort $6                           \
		--seed $7                           \
		$1                                  \
	2>> benchmark-$3-$4-$6-$NTASKS-$7-$5-$1-$2.tmp
}

#===============================================================================
#                                 MAIN ROUTINE
#===============================================================================

# Create output directory.
mkdir -p $OUTDIR

while read line
do
	strategy=`echo $line | cut -d " " -f 5`
	pdf=`echo $line | cut -d " " -f 1`
	skewness=`echo $line | cut -d " " -f 2`
	kernel=`echo $line | cut -d " " -f 4`
	seed=`echo $line | cut -d " " -f 3`
	
	run_benchmark $strategy $NTHREADS $pdf $skewness $kernel random $seed
	parse_benchmark $strategy $NTHREADS $pdf $skewness $kernel random $seed

	# House keeping.
	rm -f *.tmp
done < $DESIGN
