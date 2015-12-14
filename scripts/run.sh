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

#
# Run searcher?
#
RUN_SEARCHER=false

#
# Run simulator?
#
RUN_SIMULATOR=false

#
# Run benchmark?
#
RUN_BENCHMARK=true

# Hacked Libgomp.
LIBGOMP=$(pwd)/libsrc/libgomp/libgomp/build/.libs/

# Benchmark parameters.
LOAD=100000

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
function run_benchmark
{
	# Build thread map.
	for (( i=0; i<$1; i++ )); do
		map[$i]=$((2*$i))
	done
	
	export OMP_NUM_THREADS=$1
	export GOMP_CPU_AFFINITY="${map[@]}"

	LD_LIBRARY_PATH=$LIBGOMP \
	OMP_SCHEDULE=pedro \
	$BINDIR/benchmark --nthreads $1 --ntasks $2 --distribution $3 --niterations 1 \
	                  $4 --chunksize $5 --load $LOAD
}

# Cleanup output directory.
mkdir -p $OUTDIR
rm -f $OUTDIR/*

# Run the benchmark, simulator and searcher.
for distribution in gamma; do
	for nthreads in 12; do
		# Simulate.
		for ntasks in 128; do	
			echo $nthreads $ntasks $distribution
			run_generator $ntasks $distribution
			for chunksize in 1; do
				if [ $RUN_SIMULATOR == "true" ]; then
					run_simulator $nthreads $ntasks $distribution "static" $chunksize
					run_simulator $nthreads $ntasks $distribution "dynamic" $chunksize
				fi
				if [ $RUN_BENCHMARK == "true" ] ; then
					run_benchmark $nthreads $ntasks $distribution "static" $chunksize
					run_benchmark $nthreads $ntasks $distribution "dynamic" $chunksize
				fi
			done
			if [ $RUN_SIMULATOR == "true" ]; then
				run_simulator $nthreads $ntasks $distribution "workload-aware" 1
				run_simulator $nthreads $ntasks $distribution "smart-round-robin" 1
			fi
			if [ $RUN_BENCHMARK == "true" ] ; then
				run_benchmark $nthreads $ntasks $distribution "smart-round-robin" 1
			fi
			if [ $RUN_SEARCHER == "true" ]; then
				run_searcher $nthreads $ntasks $distribution
			fi
		done
	done
done
