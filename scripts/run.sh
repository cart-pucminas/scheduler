#
# Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

# Time utility.
TIME=/usr/bin/time

# Directories.
BINDIR=bin
OUTDIR=results

# Genetic Algorithm Parameters
NGEN=10000
POPSIZE=1000

#
# Runs the task generator.
# 
# $1 Number of tasks.
# $2 Probability distribution to use.
#
function run_generator {
	$BINDIR/generator --ntasks $1 --distribution $2 \
	1> /dev/null 2> $OUTDIR/tasks-$1-$2.out
}

#
# Runs the simulator.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
# $4 Scheduling strategy
# $5 Chunk size
#
function run_simulator {
	$BINDIR/scheduler --nthreads $1 --ntasks $2 --distribution $3 $4 --chunksize $5 \
	1> $OUTDIR/taskmap-$1-$2-$3-$4-$5.out 2> /dev/null
}

#
# Runs the searcher.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
#
function run_searcher {
	$BINDIR/searcher --nthreads $1 --ntasks $2 --distribution $3 \
	--ngen $NGEN --popsize $POPSIZE \
	1> $OUTDIR/ga-$1-$2-$3.out 2>  $OUTDIR/taskmap-$1-$2-$3-ga-1.out
}

#
# Runs the benchmark.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
# $4 Scheduling strategy
# $5 Chunk size
#
function run_benchmark {
	$TIME -f %U -o $OUTDIR/time-$1-$2-$3-$4-$5.out \
	$BINDIR/benchmark --nthreads $1 --ntasks $2 --distribution $3 $4 --chunksize $5 
}

# Cleanup output directory.
mkdir -p $OUTDIR
rm -f $OUTDIR/*

# Generate tasks.
for distribution in random normal poisson gamma beta; do
	run_generator 8192 $distribution
done

# Run the benchmark, simulator and searcher.
for distribution in random normal poisson gamma beta; do
	for nthreads in 32; do
		for ntasks in 128 256 512; do	
			echo $nthreads $ntasks $distribution
			for chunksize in 1 2 4 8 16 32; do
				run_simulator $nthreads $ntasks $distribution "workload-aware" $chunksize
				run_simulator $nthreads $ntasks $distribution "smart-round-robin" $chunksize
				run_simulator $nthreads $ntasks $distribution "static" $chunksize
				run_simulator $nthreads $ntasks $distribution "dynamic" $chunksize
				run_benchmark $nthreads $ntasks $distribution "static" $chunksize
				run_benchmark $nthreads $ntasks $distribution "dynamic" $chunksize
			done
			run_searcher $nthreads $ntasks $distribution
		done
	done
done
