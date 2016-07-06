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
SKEWNESS=(0.750 0.775 0.800 0.825 0.850 0.875 0.900)

# Kernels
KERNELS=(linear logarithm)

# Workloads.
WORKLOAD=(beta gamma gaussian poisson)

# Create directories.
rm -rf $OUTDIR
mkdir -p $OUTDIR

for workload in "${WORKLOAD[@]}";
do
	for skewness in "${SKEWNESS[@]}";
	do
		for kernel in "${KERNELS[@]}";
		do
			$BINDIR/generator              \
				--nthreads $NTHREADS       \
				--niterations $NITERATIONS \
				--pdf $workload            \
				--skewness $skewness       \
				--kernel $KERNEL           \
			2> $OUTDIR/$workload-$NITERATIONS-$skewness-$kernel.csv
		done
	done
done

