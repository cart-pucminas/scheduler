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

BINDIR=bin
OUTDIR=results
NGEN=10000
POPSIZE=1000

#
# Runs the workload generator.
# 
# $2 Number of tasks.
# $3 Probability distribution.
#
function run_generator {
	$BINDIR/generator --ntasks $1 --distribution $2 \
	2> $OUTDIR/task-$1-$2.out
	
	gnuplot -e "inputname='${OUTDIR}/task-${1}-${2}.out';\
				outputname='${OUTDIR}/task-${1}-${2}.eps'"\
				scripts/task.gp
	
	epstopdf $OUTDIR/task-$1-$2.eps
	rm $OUTDIR/task-$1-$2.eps
}

#
# Runs the static scheduler.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
#
function run_static {
	$BINDIR/scheduler --nthreads $1 --ntasks $2 --distribution $3 static \
	1> $OUTDIR/taskmap-$1-$2-$3-static.out 2> /dev/null
				
	gnuplot -e "inputname='${OUTDIR}/taskmap-${1}-${2}-${3}-static.out';\
				outputname='${OUTDIR}/taskmap-${1}-${2}-${3}-static.eps';\
				titlename='static strategy - ${3} distribution - ${2} tasks'"\
				scripts/taskmap.gp
	
	epstopdf $OUTDIR/taskmap-$1-$2-$3-static.eps
	rm $OUTDIR/taskmap-$1-$2-$3-static.eps
}

#
# Runs the static scheduler.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
#
function run_workload_aware {
	$BINDIR/scheduler --nthreads $1 --ntasks $2 --distribution $3 workload-aware \
	1> $OUTDIR/taskmap-$1-$2-$3-workload-aware.out 2> /dev/null
				
	gnuplot -e "inputname='${OUTDIR}/taskmap-${1}-${2}-${3}-workload-aware.out';\
				outputname='${OUTDIR}/taskmap-${1}-${2}-${3}-workload-aware.eps';\
				titlename='workload-aware strategy - ${3} distribution - ${2} tasks'"\
				scripts/taskmap.gp
	
	epstopdf $OUTDIR/taskmap-$1-$2-$3-workload-aware.eps
	rm $OUTDIR/taskmap-$1-$2-$3-workload-aware.eps
}

#
# Runs the dynamic scheduler.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
# $4 Chunk size
#
function run_dynamic {
	$BINDIR/scheduler --nthreads $1 --ntasks $2 --distribution $3 dynamic --chunksize $4\
	1> $OUTDIR/taskmap-$1-$2-$3-dynamic-$4.out 2> /dev/null
				
	gnuplot -e "inputname='${OUTDIR}/taskmap-${1}-${2}-${3}-dynamic-${4}.out';\
				outputname='${OUTDIR}/taskmap-${1}-${2}-${3}-dynamic-${4}.eps';\
				titlename='dynamic strategy (chunksize = ${4})- ${3} distribution - ${2} tasks'"\
				scripts/taskmap.gp
	
	epstopdf $OUTDIR/taskmap-$1-$2-$3-dynamic-$4.eps
	rm $OUTDIR/taskmap-$1-$2-$3-dynamic-$4.eps
}

#
# Runs the searcher.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
#
function run2 {
	$BINDIR/searcher --nthreads $1 --ntasks $2 --distribution $3 \
	--ngen $NGEN --popsize $POPSIZE 1> $OUTDIR/goodmap-$1-$2-$3.out \
	2>  $OUTDIR/taskmap-$1-$2-$3-ga.out
	
	gnuplot -e "titlename='${2} ${3}';\
	            inputname='${OUTDIR}/goodmap-${1}-${2}-${3}.out';\
				inputname2='${OUTDIR}/taskmap-${1}-${2}-${3}-static.out';\
				inputname3='${OUTDIR}/taskmap-${1}-${2}-${3}-dynamic-1.out';\
				inputname4='${OUTDIR}/taskmap-${1}-${2}-${3}-dynamic-2.out';\
				inputname5='${OUTDIR}/taskmap-${1}-${2}-${3}-dynamic-4.out';\
				inputname6='${OUTDIR}/taskmap-${1}-${2}-${3}-dynamic-8.out';\
				inputname7='${OUTDIR}/taskmap-${1}-${2}-${3}-workload-aware.out';\
				outputname='${OUTDIR}/goodmap-${1}-${2}-${3}.eps';\
				titlename='${3} distribution - ${2} tasks'"\
				scripts/goodmap.gp
				
	gnuplot -e "inputname='${OUTDIR}/taskmap-${1}-${2}-${3}-ga.out';\
				outputname='${OUTDIR}/taskmap-${1}-${2}-${3}-ga.eps';\
				titlename='GA - ${2} tasks'"\
				scripts/taskmap.gp
	
	epstopdf $OUTDIR/goodmap-$1-$2-$3.eps
	rm $OUTDIR/goodmap-$1-$2-$3.eps
}

mkdir -p $OUTDIR
rm -f $OUTDIR/*

# Generates workload.
for distribution in random normal poisson gamma beta; do
	run_generator 8192 $distribution
done

for distribution in random normal poisson gamma beta; do
	for nthreads in 32; do
		for ntasks in 128 256 512; do	
			echo $nthreads $ntasks $distribution
			run_static $nthreads $ntasks $distribution
			run_workload_aware $nthreads $ntasks $distribution
			for chunksize in 1 2 4 8 16; do
				run_dynamic $nthreads $ntasks $distribution $chunksize
			done
			run2 $nthreads $ntasks $distribution
		done
	done
done
