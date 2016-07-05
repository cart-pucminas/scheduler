#
# Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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
# Program arguments.
#   $1: Number of threads.
#   $2: Number of loop iterations.
#
NTHREADS=$1    # Number of threads.
NITERATIONS=$2 # Number of iterations.

# Directories.
BINDIR=$PWD/bin
OUTDIR=$PWD/input

# Skewness
SKEWNESS=(0.50 0.55 0.60 0.65 0.70 0.75 0.80 0.85 0.90)

# Workloads.
WORKLOAD=(beta gamma gaussian poisson)

# Create directories.
rm -rf $OUTDIR
mkdir -p $OUTDIR

for workload in "${WORKLOAD[@]}"; do
	for skewness in "${SKEWNESS[@]}"; do
		$BINDIR/generator              \
			--nthreads $NTHREADS       \
			--niterations $NITERATIONS \
			--pdf $workload            \
			--skewness $skewness       \
		2> $OUTDIR/$workload-$NITERATIONS-$skewness.csv
	done
done

