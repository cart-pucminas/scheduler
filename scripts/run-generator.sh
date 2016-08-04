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
#   $1: Number of tasks.
#   $2: Number of task classes.
#   $3: Output directory.
#
NTASKS=$1   # Number of tasks.
NCLASSES=$2 # Number of task classes.
OUTDIR=$3   # Output directory.

# Directories
BINDIR=$PWD/bin

# Import some variables.
source scripts/var.sh

# Create directories.
mkdir -p $OUTDIR

# Run workload generator.
for workload in "${WORKLOAD[@]}";
do
	for skewness in "${SKEWNESS[@]}";
	do
		for kernel in "${KERNELS[@]}";
		do
			$BINDIR/generator                                            \
				--nclasses $NCLASSES                                     \
				--ntasks $NTASKS                                         \
				--pdf $workload                                          \
				--skewness $skewness                                     \
				--kernel $kernel                                         \
			2> $OUTDIR/$workload-$NTASKS-$skewness-$kernel-histogram.csv \
			1> $OUTDIR/$workload-$NTASKS-$skewness-$kernel.csv
		done
	done
done

