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
export BINDIR = $(CURDIR)/bin
export INCDIR = $(CURDIR)/include
export SRCDIR = $(CURDIR)/src

# Toolchain
export CC = gcc
export LD = gcc

# Toolchain configuration.
export CFLAGS += -I $(INCDIR)
export CFLAGS += -std=c99 -pedantic
export CFLAGS += -Wall -Wextra -Werror
export CFLAGS += -O3

# Libraries.
export LIBS = -lm

# Source files.
SRC = $(wildcard $(SRCDIR)/*.c)

# Object files.
OBJ = $(SRC:.c=.o)

# Builds everything
all: workloadgen

# Builds WorkloadGen.
workloadgen:               \
	$(SRCDIR)/statistics.o \
	$(SRCDIR)/workload.o

# Builds object file from C source file.
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# Cleans compilation files.
clean:
	@rm -f $(OBJ)
