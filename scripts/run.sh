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
# Number of threads.
#
NTHREADS=12

#
# Simultaneos Multithreading?
#
SMT=true

#
# Kernel class.
#
CLASS=small

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

#
# Run kernels?
#
RUN_KERNELS=false

# Hacked Libgomp.
LIBGOMP=$(pwd)/libsrc/libgomp/libgomp/build/.libs/

# Benchmark parameters.
LOAD=200000000

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
# Maps threads on the cores.
#
# $1 Number of threads.
#
function map_threads
{
	# Build thread map.
	if [ $SMT == "true" ]; then
		for (( i=0; i<$1; i++ )); do
			map[$i]=$((2*$i))
		done
	else
		for (( i=0; i<$1; i++ )); do
			map[$i]=$i
		done
	fi
	
	export OMP_NUM_THREADS=$1
	export GOMP_CPU_AFFINITY="${map[@]}"
}

#
# Runs the benchmark.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
# $4 Scheduling strategy
# $5 Chunk size
# $6 Iteration
# $7 Seed.
#
function run_benchmark
{
	map_threads $1

	GSL_RNG_SEED=$7          \
	LD_LIBRARY_PATH=$LIBGOMP \
	OMP_SCHEDULE=pedro \
	$BINDIR/benchmark --nthreads $1 --ntasks $2 --distribution $3 --niterations 1 \
	                  $4 --chunksize $5 --load $LOAD | tail -n 1
#					  >> $OUTDIR/$3-$2-$1-$4-$5-$6.out \
#					  2> /dev/null
}

#
# Runs the kernel.
#
# $1 Kernel.
# $2 Number of threads.
# $3 Scheduling strategy.
#
function run_kernel
{
	map_threads $2

	LD_LIBRARY_PATH=$LIBGOMP \
	OMP_SCHEDULE=pedro \
	$BINDIR/$1.$3 --class $CLASS --nthreads $2 \
#	>> $OUTDIR/$1-$3-$CLASS-$2.out \
#	2> $OUTDIR/$CLASS-$1.tasks
}

# Cleanup output directory.
mkdir -p $OUTDIR
rm -f $OUTDIR/*

# 7 113 [47] 91 [3]

# Run the benchmark, simulator and searcher.
for ntasks in 96; do	
#	for i in {1..5}; do
	for seed in {1..100}; do
		echo seed $seed
		for distribution in random; do
			for nthreads in 12; do
				# Simulate.
				run_generator $ntasks $distribution
				for chunksize in 1; do
					if [ $RUN_SIMULATOR == "true" ]; then
						run_simulator $nthreads $ntasks $distribution "static" $chunksize
						run_simulator $nthreads $ntasks $distribution "dynamic" $chunksize
					fi
					if [ $RUN_BENCHMARK == "true" ] ; then
#						run_benchmark $nthreads $ntasks $distribution "static" $chunksize $i $seed
						run_benchmark $nthreads $ntasks $distribution "dynamic" $chunksize $i $seed
					fi
				done
				if [ $RUN_SIMULATOR == "true" ]; then
					run_simulator $nthreads $ntasks $distribution "workload-aware" 1
					run_simulator $nthreads $ntasks $distribution "smart-round-robin" 1
				fi
				if [ $RUN_BENCHMARK == "true" ] ; then
					run_benchmark $nthreads $ntasks $distribution "smart-round-robin" 1 $i $seed
				fi
				if [ $RUN_SEARCHER == "true" ]; then
					run_searcher $nthreads $ntasks $distribution
				fi
			done
		done
#		done
	done
	if [ $RUN_KERNELS == "true" ]; then
		for kernel in is; do
			for scheduler in static dynamic srr; do
				run_kernel $kernel $NTHREADS $scheduler
			done
		done
	fi
done

