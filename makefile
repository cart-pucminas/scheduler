#
# Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
# 
# This file is part of WorkloadGen.
#
# WorkloadGen is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
# 
# WorkloadGen is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with WorkloadGen; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#

# Directories.
export BINDIR =  $(CURDIR)/bin
export CONTRIB = $(CURDIR)/contrib
export INCDIR =  $(CURDIR)/include
export LIBDIR =  $(CURDIR)/lib
export SRCDIR =  $(CURDIR)/src

# Toolchain
export AR = ar
export CC = gcc
export LD = gcc

# Toolchain configuration.
export ARFLAGS  = -vq
export CFLAGS  += -I $(INCDIR)
export CFLAGS  += -std=c99 -pedantic -D_XOPEN_SOURCE
export CFLAGS  += -Wall -Wextra -Werror
export CFLAGS  += -O3

# Libraries.
export LIBS = $(LIBDIR)/libmy.a -lm

# Builds everything
all: workloadgen simsched

# Builds SimSched
simsched: mylib
	cd $(SRCDIR) && $(MAKE) simsched

# Builds WorkloadGen.
workloadgen: mylib
	cd $(SRCDIR) && $(MAKE) workloadgen

# Builds MyLib:
mylib:
	cd $(CONTRIB) && $(MAKE) all

# Cleans compilation files.
clean:
	cd $(CONTRIB) && $(MAKE) clean
	cd $(SRCDIR) && $(MAKE) clean
