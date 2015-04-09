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

#
# Runs the scheduler.
# 
# $1 Number of threads
# $2 Number of tasks.
# $3 Probability distribution.
# $4 Scheduling strategy.
#
function run {
	$BINDIR/scheduler --nthreads $1 --ntasks $2 --distribution $3 $4 \
	1>> $OUTDIR/scheduler-$1-$2-$3-$4.out 2>> $OUTDIR/scheduler-$1-$2-$3-$4.err
}

rm -fr $OUTDIR
mkdir -p $OUTDIR

for strategy in static; do
	for distribution in random normal; do
		for nthreads in 2 4 8 16 32 64 128 256; do
			for ntasks in 1024 2048 4096 8192; do
				run $nthreads $ntasks $distribution $strategy
			done
		done
	done
done
