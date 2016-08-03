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

# Workloads
WORKLOAD=(beta gamma gaussian)

# Skewness
SKEWNESS=(0.825 0.840 0.855 0.870 0.885 0.900)

# Kernels
KERNELS=(linear logarithm quadratic)


# Scheduling strategies.
STRATEGIES=(static dynamic workload-aware srr best)


# Workload sorting.
SORT=(random)
