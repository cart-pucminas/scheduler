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

# Results directory.
RESULTS=results

# Scripts directory.
SCRIPTS=scripts

# Not used.
NTHREADS=12
CHUNKSIZE=1

mkdir -p $RESULTS/tasks
mkdir -p $RESULTS/benchmark/time
mkdir -p $RESULTS/benchmark/thread
mkdir -p $RESULTS/simulator/time
mkdir -p $RESULTS/simulator/thread

#
# Extracts information from benchmark trace file.
#
for distribution in beta gamma normal poisson random; do
	for ntasks in 48 96 192; do
		prefix1=$RESULTS/$distribution-$ntasks
		for seed in {1..30}; do
			tail -n $ntasks $prefix1-$seed.tasks \
				> $RESULTS/tasks/$distribution-$ntasks-$seed.tasks.csv
			
			rm $prefix1-$seed.tasks
		done
		for scheduler in dynamic static smart-round-robin; do
			for seed in {1..30}; do
				prefix2=$prefix1-$seed-$scheduler-$CHUNKSIZE-$NTHREADS

				tail -n 1 $prefix2.benchmark \
					>> $prefix1-$scheduler.tmp1

				tail -n 1 $prefix2.simulator \
					>> $prefix1-$scheduler.tmp2

				head -n $NTHREADS $prefix2.benchmark | cut -d" " -f 3,4 \
					> $prefix1-$seed-$scheduler.csv
				mv $RESULTS/*.csv $RESULTS/benchmark/thread

				head -n $NTHREADS $prefix2.simulator | cut -d";" -f 2 \
					> $prefix1-$seed-$scheduler.csv
				mv $RESULTS/*.csv $RESULTS/simulator/thread
				
				rm $prefix2.simulator
				rm $prefix2.benchmark
			done
			cut -d" " -f 2 $prefix1-$scheduler.tmp1 \
				>> $prefix1-$scheduler.time

			rm $RESULTS/*.tmp1
		done
		paste -d";"                         \
			$prefix1-static.tmp2            \
			$prefix1-dynamic.tmp2           \
			$prefix1-smart-round-robin.tmp2 \
			>> $RESULTS/simulator/time/$distribution-$ntasks.csv

		paste -d";"                         \
			$prefix1-dynamic.time           \
			$prefix1-smart-round-robin.time \
			>> $RESULTS/benchmark/time/$distribution-$ntasks.csv
		
		rm $prefix1-*.tmp2
		rm $prefix1-*.time
	done
done
