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

# Directories.
export BINDIR    = $(CURDIR)/bin
export DOCDIR    = $(CURDIR)/doc
export INCDIR    = $(CURDIR)/include
export LIBDIR    = $(CURDIR)/lib
export LIBSRCDIR = $(CURDIR)/libsrc
export SRCDIR    = $(CURDIR)/src

# Libraries.
export LIBS = $(LIBDIR)/libmy.a -lm

# Toolchain.
export CC = gcc

# Toolchain configuration.
export CFLAGS  = -I $(INCDIR)
export CFLAGS += -std=c99 -pedantic -Wall -Wextra -Werror -fopenmp
export CFLAGS += -O3

# Builds everything.
all: simulator searcher

# Builds libraries.
libs:
	mkdir -p $(LIBDIR)
	cd $(LIBSRCDIR) && $(MAKE) all

# Builds the simulator.
simulator: libs
	mkdir -p $(BINDIR)
	cd $(SRCDIR) && $(MAKE) simulator

# Builds the searcher.
searcher: libs
	mkdir -p $(BINDIR)
	cd $(SRCDIR) && $(MAKE) searcher
	
# Cleans compilation files.
clean:
	cd $(LIBSRCDIR) && $(MAKE) clean
	cd $(SRCDIR) && $(MAKE) clean
