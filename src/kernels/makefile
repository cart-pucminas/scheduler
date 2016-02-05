#
# Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com
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

all: static dynamic smart-round-robin

# Builds static kernels.
static:
	cd fn-kernel && $(MAKE) static
	cd is-kernel && $(MAKE) static
	cd km-kernel && $(MAKE) static

# Builds dynamic kernels.
dynamic:
	cd fn-kernel && $(MAKE) dynamic
	cd is-kernel && $(MAKE) dynamic
	cd km-kernel && $(MAKE) dynamic

# Builds runtime kernels.
smart-round-robin:
	cd fn-kernel && $(MAKE) smart-round-robin
	cd is-kernel && $(MAKE) smart-round-robin

# Cleans compilation files.
clean:
	cd fn-kernel && $(MAKE) clean
	cd is-kernel && $(MAKE) clean
	cd km-kernel && $(MAKE) clean

