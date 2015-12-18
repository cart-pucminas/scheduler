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
mkdir -p $RESULTS/time
mkdir -p $RESULTS/additions
mkdir -p $RESULTS/thread

#
# Extracts information from benchmark trace file.
#
#
for distribution in random normal beta gamma poisson; do
	for ntasks in 48 96; do
		prefix1=$RESULTS/$distribution-$ntasks
		for seed in {1..30}; do
			tail -n $ntasks $prefix1-$seed.tasks \
				> $RESULTS/tasks/$distribution-$ntasks-$seed.tasks.csv
			
			rm $prefix1-$seed.tasks
		done
		for scheduler in dynamic smart-round-robin; do
			for seed in {1..30}; do
				prefix2=$prefix1-$seed-$scheduler-$CHUNKSIZE-$NTHREADS

				tail -n 1 $prefix2.benchmark \
					>> $prefix1-$scheduler.tmp

				head -n $NTHREADS $prefix2.benchmark | cut -d" " -f 3,4 \
					> $prefix1-$seed-$scheduler.thread.csv
				mv $RESULTS/*.thread.csv $RESULTS/thread

				rm $prefix2.benchmark
			done
			cut -d" " -f 2 $prefix1-$scheduler.tmp \
				>> $prefix1-$scheduler.time

			cut -d" " -f 3 $prefix1-$scheduler.tmp \
				>> $prefix1-$scheduler.additions

			rm $RESULTS/*.tmp
		done
		paste -d";"                         \
			$prefix1-dynamic.time           \
			$prefix1-smart-round-robin.time \
			>> $RESULTS/time/$distribution-$ntasks.time.csv

		paste -d";"                              \
			$prefix1-dynamic.additions           \
			$prefix1-smart-round-robin.additions \
			>> $RESULTS/additions/$distribution-$ntasks.additions.csv
		
		rm $prefix1-*.time
		rm $prefix1-*.additions
	done
done
