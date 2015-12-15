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
RESULTS_DIR=results

# Not used.
NTHREADS=12
CHUNKSIZE=1

for it in {i..10}; do
	for distribution in beta normal; do
		$prefix1=$RESULTS/$ditribution
		for ntasks in 512; do
			prefix2=$prefix1-$ntasks-$NTHREADS
			for scheduler in static dynamic smart-round-robin; do
				file1=$prefix2-$scheduler-$CHUNKSIZE-$it.out
				file2=$prefix2-$scheduler-$CHUNKSIZE.time
				file3=$prefix2-$scheduler-$CHUNKSIZE-$it.imbalance.csv
				tail -n $((1 + $NTHREADS)) $file >> $file2
				head -n $NTHREADS          $file >  $file3
			done
			paste -d";" \
				$prefix2-static-$CHUNKSIZE.time \
				$prefix2-dynamic-$CHUNKSIZE.time \
				$prefix2-smart-round-robin-$CHUNKSIZE.time \
				>> $prefix2.tmp
				rm *.time
		done
		paste -d";" \
			$prefix1-128-$NTHREADS.tmp \
			$prefix1-256-$NTHREADS.tmp \
			$prefix1-512-$NTHREADS.tmp \
			>> $prefix1.csv
			rm *.tmp
	done
done
