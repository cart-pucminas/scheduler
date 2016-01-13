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
NTHREADS=4

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
RUN_BENCHMARK=false

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
# $3 Seed to use.
#
function run_generator
{
	GSL_RNG_SEED=$3                                 \
	$BINDIR/generator --ntasks $1 --distribution $2 \
		1> /dev/null 2> $OUTDIR/$2-$1-$3.tasks
}

#
# Runs the simulator.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
# $4 Scheduling strategy
# $5 Chunk size
# $6 Seed.
#
function run_simulator
{
	GSL_RNG_SEED=$6                                                                 \
	$BINDIR/scheduler --nthreads $1 --ntasks $2 --distribution $3 $4 --chunksize $5 \
		1> $OUTDIR/$3-$2-$6-$4-$5-$1.simulator \
		2> /dev/null
}

#
# Runs the searcher.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
# #4 Seed.
#
function run_searcher
{
	GSL_RNG_SEED=$4                                              \
	$BINDIR/searcher --nthreads $1 --ntasks $2 --distribution $3 \
		--ngen $NGEN --popsize $POPSIZE                          \
		1> $OUTDIR/$3-$2-$4-$1.info.searcher                     \
		2> $OUTDIR/$3-$2-$4-$1.taskmap.searcher
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
# $6 Seed.
#
function run_benchmark
{
	map_threads $1

	GSL_RNG_SEED=$6                                                               \
	LD_LIBRARY_PATH=$LIBGOMP                                                      \
	OMP_SCHEDULE=pedro                                                            \
	$BINDIR/benchmark --nthreads $1 --ntasks $2 --distribution $3 --niterations 1 \
		$4 --chunksize $5 --load $LOAD                                            \
		>> $OUTDIR/$3-$2-$6-$4-$5-$1.benchmark                                    \
		2> /dev/null
}

#
# Runs the kernel.
#
# $1 Kernel.
# $2 Number of threads.
# $3 Scheduling strategy.
# #4 Seed.
#
function run_kernel
{
	map_threads $2
	
	GSL_RNG_SEED=$4                            \
	LD_LIBRARY_PATH=$LIBGOMP                   \
	OMP_SCHEDULE=pedro                         \
	$BINDIR/$1.$3 --class $CLASS --nthreads $2 \
		>> $OUTDIR/$1-$4-$3-$CLASS-$2.out      \
		2> $OUTDIR/$1-$4-$3-$CLASS-$2.tasks
}

# Run the benchmark, simulator and searcher.
for it in {1..10}; do
	
	rm -rf $OUTDIR
	mkdir -p $OUTDIR
	
	for seed in {1..20}; do
		for (( nthreads=1; nthreads<=$NTHREADS; nthreads++ )); do
			for ntasks in 48 96 192; do
				for distribution in beta gamma normal poisson random; do
					for chunksize in 1 2 4; do
						if [ $RUN_SIMULATOR == "true" ]; then
							run_simulator $nthreads $ntasks $distribution "static" $chunksize $seed
							run_simulator $nthreads $ntasks $distribution "dynamic" $chunksize $seed
						fi
						if [ $RUN_BENCHMARK == "true" ] ; then
							run_benchmark $nthreads $ntasks $distribution "static" $chunksize $seed
							run_benchmark $nthreads $ntasks $distribution "dynamic" $chunksize $seed
						fi
					done
					if [ $RUN_SIMULATOR == "true" ]; then
						run_simulator $nthreads $ntasks $distribution "workload-aware" 1 $seed
						run_simulator $nthreads $ntasks $distribution "smart-round-robin" 1 $seed
					fi
					if [ $RUN_BENCHMARK == "true" ] ; then
						run_benchmark $nthreads $ntasks $distribution "smart-round-robin" 1 $seed
					fi
					if [ $RUN_SEARCHER == "true" ]; then
						run_searcher $nthreads $ntasks $distribution $seed
					fi
				done
			done
			if [ $RUN_KERNELS == "true" ]; then
				for kernel in is km; do
					for scheduler in static dynamic srr; do
						run_kernel $kernel $nthreads $scheduler $seed
					done
				done
			fi
		done
	done
	
	tar -cjvf $OUTDIR-$it.tar.bz2 $OUTDIR/*
done

