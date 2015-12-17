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

# Not used.
NTHREADS=12
CHUNKSIZE=1

#
# Extracts information from benchmark trace file.
#
#
for distribution in random normal beta gamma poisson; do
	for ntasks in 48 96 192; do
		prefix1=$RESULTS/$distribution-$ntasks
		for scheduler in static dynamic smart-round-robin; do
			for seed in {1..30}; do
				prefix2=$prefix1-$seed-$CHUNKSIZE-$NTHREADS
				head -n $NTHREADS \
					> $RESULTS/$prefix-$seed-$scheduler.thread
				tail -n 1 $prefix2.benchmark \
					>> $RESULTS/$prefix-$scheduler.tmp
			done
			cut -d" " -f 2 $RESULTS/$prefix1-$scheduler.tmp \
				>> $RESULTS/$prefix1.time
			cut -d" " -f 3 $RESULTS/$prefix1-$scheduler.tmp \
				>> $RESULTS/$prefix1.additions
			rm $RESULTS/*tmp
		done
	done
done
